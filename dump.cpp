#include "dump.h"

namespace dump
{

Dumper::Dumper(DumperSettings settings)
{
    SetSettings(std::move(settings));
    ClearLine();
}

void Dumper::SetSettings(DumperSettings settings)
{
    m_settings = std::move(settings);

//    m_rangeHKeys.from.value = m_settings.hkeyFrom;
//    m_rangeHKeys.till.value = m_settings.hkeyTill;

}

void Dumper::ClearLine()
{
    m_ctx.line = Line();
    if (m_settings.isArray)
    {
        m_ctx.line.offset = "//";
        m_ctx.line.ascii  = "// ";
    }
}

void Dumper::Generate(void* bufferVoid, size_t bufferLength)
{
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    buffer = FindKeysAndShiftStartOfBuffer(bufferLength, buffer);

    utilities::PrintSeparator();

    GenerateImpl(buffer);

    utilities::PrintSeparator();

    utilities::PrintRange(m_settings.range, m_settings.isShowDetail, m_settings.shift);

    for (const auto& key : m_keys)
        utilities::PrintFoundKeyResults(key, m_settings.isShowDetail, m_settings.shift);
}

void Dumper::GenerateImpl(uint8_t* buffer)
{
//    std::string currentWord;
    IsVisible charIsVisible;

    PrepareFirstLine(m_settings.range.begin);

    for (size_t index = m_settings.range.begin; index <= m_settings.range.end; ++index)
    {
        if (PositionInLine(index) == 0)
        {
            m_ctx.range.begin = index;
            m_ctx.line.offset.append(GetOffset(index));
        }

        charIsVisible = IsCharVisible(buffer, index);

        if (IsNextWordStart(index, charIsVisible) && m_settings.useNewLine)
            PrintLineAndIntend(index);

        const auto currentByte = buffer[index];
        
        utilities::Color currentColor = GetColor(index, currentByte);

        AppendCurrentDumpLine(currentByte, index, currentColor);

        AppendCurrentAsciiLine(currentByte, index, charIsVisible, currentColor);

        m_ctx.range.end = index;

        if (IsEndOfCurrentLine(index))
        {
            CompleteCurrentDumpLine();

            bool currentLineIsSkipped = IsLineSkipped(m_ctx.range);
            bool isNewGroupAfterSkippedLines = !currentLineIsSkipped && m_ctx.previousLineWasSkipped;

            if (!currentLineIsSkipped)
                PrintAndClearLine(isNewGroupAfterSkippedLines);
            else
                ClearLine();

            if (currentLineIsSkipped)
                ++m_ctx.skipLinesCount;
            else
                m_ctx.skipLinesCount = 0;

            m_ctx.previousLineWasSkipped = currentLineIsSkipped;
        }
    }
    if (m_settings.isArray)
        std::cout << "};\n";
}

uint8_t* Dumper::FindKeysAndShiftStartOfBuffer(const size_t bufferLength, uint8_t* buffer)
{
    if ((m_settings.range.end == 0 && m_settings.range.begin != 0) ||
        (m_settings.range.end >= bufferLength))
    {
        m_settings.range.end = bufferLength - 1;
    }

    finder::HexKeyFinder hexKeyFinder;

    m_settings.range = hexKeyFinder.FindRange(buffer, m_settings.range, m_settings.hkeyFrom, m_settings.hkeyTill);

    if (m_settings.countBytesAfterHkeyFrom != 0)
    {
        const size_t expectedEnd = m_settings.range.begin + m_settings.countBytesAfterHkeyFrom - 1;
        if (expectedEnd < m_settings.range.end)
            m_settings.range.end = expectedEnd;
    }

    if (m_settings.useRelativeAddress)
        buffer = ShiftBeginOfBufferAndResults(buffer, m_settings);

    // add --from and --till
    AddKeyInVector(m_settings.hkeyFrom, m_keys);
    AddKeyInVector(m_settings.hkeyTill, m_keys);

    //add keys
    for (const auto& keyValue : m_settings.keyValues)
        m_keys.push_back(std::make_shared<finder::Key>(m_keys.size(), keyValue, std::make_shared<finder::KeyFinder>()));

    // add hex keys
    for (const auto& hkeyValue : m_settings.hkeyValues)
        AddKeyInVector(hkeyValue, m_keys);

    // search // TODO for :
    for (size_t index = 0; index < m_keys.size(); ++index)
    {
        auto key = m_keys[index];
        key->results = key->Find(buffer, m_settings.range, *key);
    }

    return buffer;
}

void Dumper::AddKeyInVector(const std::string& keyValue, finder::SharedKeysVector& keysVector) const
{
    if (keyValue.empty())
        return;
    keysVector.push_back(std::make_shared<finder::Key>(keysVector.size(), keyValue));
}

utilities::Color Dumper::GetColor(size_t index, const uint8_t currentValue) const
{
    for (const auto& key : m_keys)
        if (!key->results.empty())
            for (const auto position : key->results)
                if (index - position < key->length)
                    return utilities::GetColorIndex(key->id);

    if (currentValue == 0 && m_settings.printZeroAsGrey)
        return utilities::Color::Grey;

    return utilities::Color::Normal;
}

bool Dumper::IsLineSkipped(Range range)
{
    if (!m_settings.skipTextWithoutKeys)
        return false;

    for (const auto& key : m_keys)
        for (const auto index : key->results)
            if (IsLineNearKeys(range, index, key->length))
                return false;

    return true;
}

bool Dumper::IsLineNearKeys(const Range& range, size_t position, size_t keySize) const
{
    return (range.begin <= position  + m_settings.countBytesAfterKey + keySize - 1) &&
              (position <= range.end + m_settings.countBytesBeforeKey);
}

uint8_t* Dumper::ShiftBeginOfBufferAndResults(uint8_t* buffer, DumperSettings& settings) // TODO only range and shift
{
//    if (!m_rangeHKeys.from.results.empty())
//    {
//        const auto shift = m_rangeHKeys.from.results.front();
//        for (auto& index : m_rangeHKeys.from.results)
//            index -= shift;
//        for (auto& index : m_rangeHKeys.till.results)
//            index -= shift;
//    }
//
//    settings.shift = settings.range.begin;

    settings.shift = settings.range.begin;
    settings.range.begin = 0;
    settings.range.end -= settings.shift;

    return buffer + settings.shift;
}

void Dumper::CompleteCurrentDumpLine()
{
    if (!m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
}

void Dumper::PrepareFirstLine(size_t index) // Actual for --shift=true
{
    if (m_settings.isArray)
        std::cout << "{\n";

    if (PositionInLine(m_settings.range.begin) == 0)
        return;

    m_ctx.line.offset += GetOffset(index);

    const auto positionInLine = PositionInLine(index);
    m_ctx.line.dump   += GetSpacesForBeginOfDumpLine (positionInLine);
    m_ctx.line.ascii  += GetSpacesForBeginOfAsciiLine(positionInLine);
}

void Dumper::AppendCurrentDumpLine(uint8_t dumpByte, const size_t index, utilities::Color currentColor)
{
    if (currentColor != utilities::Color::Normal)
        m_ctx.line.dump.append(utilities::ColorAnsiCode[currentColor]);

    m_ctx.line.dump.append(GetDumpValueSymbol(dumpByte));

    if (currentColor != utilities::Color::Normal)
        m_ctx.line.dump.append(utilities::ColorAnsiCode[utilities::Color::Normal]);

    if (m_settings.isArray && index < m_settings.range.end)
        m_ctx.line.dump.append(",");

    if (m_settings.isArray && index == m_settings.range.end)
        m_ctx.line.dump.append(" ");

    m_ctx.line.dump.append(" ");

    if (IsPositionLastInColumn(index) && m_settings.isSpaceBetweenDumpBytes)
        m_ctx.line.dump.append(" ");

    if (index == m_settings.range.end)
        m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index)));
}

void Dumper::AppendCurrentAsciiLine(uint8_t dumpByte, size_t index, const IsVisible& isVisible, utilities::Color currentColor)
{
    if (currentColor != utilities::Color::Normal)
        m_ctx.line.ascii.append(utilities::ColorAnsiCode[currentColor]);

    if (isVisible.current)
    {
        m_ctx.line.ascii += static_cast<char>(dumpByte);
//        currentWord += buffer[index];
    }
    else
    {
        if (m_settings.useWideChar && isVisible.previous && isVisible.next)
        {
            m_ctx.line.ascii += m_settings.widePlaceHolder;
        }
        else
        {
            if (dumpByte == 0)
                m_ctx.line.ascii += m_settings.zeroPlaceHolder;
            else
                m_ctx.line.ascii += m_settings.placeHolder;
        }
    }

    if (IsPositionLastInColumn(index) && m_settings.isSpaceBetweenAsciiBytes)
        m_ctx.line.ascii.append(" ");

    if (currentColor != utilities::Color::Normal)
        m_ctx.line.ascii.append(utilities::ColorAnsiCode[utilities::Color::Normal]);
}

void Dumper::PrintLineAndIntend(size_t index)
{
    m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index - 1)));
    if (!m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
    PrintAndClearLine(false);

    m_ctx.line.offset = GetOffset(index);
    m_ctx.line.indent = GetSpacesForBeginOfDumpLine(PositionInLine(index));
    if (m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
}

bool Dumper::IsEndOfCurrentLine(size_t index) const
{
    return (PositionInLine(index) == m_settings.bytesInLine - 1) || (index == m_settings.range.end);
}

bool Dumper::IsNextWordStart(size_t index, const IsVisible& isVisible) const
{
    return (index != 0) && isVisible.current &&
            ((!m_settings.useWideChar && !isVisible.previous) ||
              (m_settings.useWideChar && !isVisible.previous && !isVisible.preprevious));
}

IsVisible Dumper::IsCharVisible(const uint8_t* buffer, const size_t index) const
{
    IsVisible isVisible;

    isVisible.preprevious = index > 1 && isprint(buffer[index - 2]);
    isVisible.previous    = index > 0 && isprint(buffer[index - 1]);
    isVisible.current     =              isprint(buffer[index]);
    isVisible.next        = index < m_settings.range.end && isprint(buffer[index + 1]);

    return isVisible;
}

std::string Dumper::GetDumpValueSymbol(uint8_t value) const
{
    std::string dumpValueStr;

    if (m_settings.isArray)
        dumpValueStr.append("0x");

    const auto dumpValue = boost::format("%02x") % static_cast<uint16_t>(value);

    dumpValueStr += dumpValue.str();

    return dumpValueStr;
}

size_t Dumper::PositionInLine(size_t index) const
{
    return index % m_settings.bytesInLine;
}

bool Dumper::IsPositionLastInColumn(size_t index) const
{
    return index % m_settings.bytesInGroup == m_settings.bytesInGroup - 1;
}

std::string Dumper::GetSpacesForBeginOfDumpLine(const size_t positionInLine) const
{
    if (positionInLine == 0)
        return "";

    size_t lengthOfBeginOfLine = dump::size::CharsForOneByte * positionInLine;

    if (m_settings.isSpaceBetweenDumpBytes)
        lengthOfBeginOfLine += positionInLine / m_settings.bytesInGroup;

    if (m_settings.isArray)
        lengthOfBeginOfLine += dump::size::AdditionalCharsForCArray * positionInLine;

    std::string spaces(lengthOfBeginOfLine, ' ');

    return spaces;
}

std::string Dumper::GetSpacesForBeginOfAsciiLine(const size_t positionInLine)
{
    size_t lengthOfBeginOfLine = positionInLine;

    if (m_settings.isSpaceBetweenAsciiBytes)
        lengthOfBeginOfLine += (positionInLine) / m_settings.bytesInGroup;

    std::string spaces(lengthOfBeginOfLine, ' ');

    return spaces;
}

std::string Dumper::GetSpacesForRestOfDumpLine(const size_t positionInLine) const
{
    const auto restBytesInLine = m_settings.bytesInLine - positionInLine - 1;

    size_t lengthOfRestLine = dump::size::CharsForOneByte * restBytesInLine;
    if (m_settings.isSpaceBetweenDumpBytes)
        lengthOfRestLine += (restBytesInLine + m_settings.bytesInGroup - 1) / m_settings.bytesInGroup;

    if (m_settings.isArray)
        lengthOfRestLine += dump::size::AdditionalCharsForCArray * restBytesInLine;

    std::string spaces(lengthOfRestLine, ' ');

    return spaces;
}


std::string Dumper::GetOffset(size_t i) const
{
    std::string result;

    if (m_settings.offset == OffsetTypes::Dec || m_settings.offset == OffsetTypes::Both)
    {
        if (m_settings.shift)
            result += (boost::format("%5d") % i).str();

        if (m_settings.shift && m_settings.isShowDetail)
            result += "/";

        if (m_settings.isShowDetail || !m_settings.shift)
            result += (boost::format("%5d") % (m_settings.shift + i)).str();
    }

    if (m_settings.offset == OffsetTypes::Both)
        result += "'";

    if (m_settings.offset == OffsetTypes::Hex || m_settings.offset == OffsetTypes::Both)
    {
        if (m_settings.shift)
            result += (boost::format("0x%04x") % (i)).str();

        if (m_settings.shift && m_settings.isShowDetail)
            result += "/";

        if (m_settings.isShowDetail || !m_settings.shift)
            result += (boost::format("0x%04x") % (m_settings.shift + i)).str();
    }

    if (m_settings.offset != OffsetTypes::None)
        result += ": ";

    return result;
}


void Dumper::PrintAndClearLine(const bool isNewGroupAfterSkippedLines)
{
    if (isNewGroupAfterSkippedLines)
        utilities::PrintEmptyLine(m_settings.isShowEmptyLines, m_ctx.skipLinesCount);

    m_ctx.line.debug = "<" + std::to_string(m_ctx.range.begin) + "..." + std::to_string(m_ctx.range.end) + "> ";

    m_ctx.Print(m_settings.isShow, m_settings.isArray);

    ClearLine();
}

void Dumper::Ctx::Print(const DumperSettings::IsShow& isShow, bool isArray)
{
    if (isShow.offset && !isArray)
        std::cout << line.offset;

    if (isShow.dump)
        std::cout << line.dump;

    if (isShow.offset && isArray)
        std::cout << line.offset;

    if (isShow.ascii)
        std::cout << line.ascii;

    if (isShow.debug)
        std::cout << "\t" << line.debug;

    std::cout << std::endl;
}
} // namespace dump
