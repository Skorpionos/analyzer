#include <set>
#include "dump.h"
#include "finder.h"

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

    m_keys.key.value      = m_settings.key;
    m_keys.hkeyFrom.value = m_settings.hkeyFrom;
    m_keys.hkeyTill.value = m_settings.hkeyTill;

}
void Dumper::AddKeyInVector(TheKey& key, std::vector<TheKey*, std::allocator<TheKey*>>& keysPtrs)
{
    if (!key.value.empty())
    {
        key.id = keysPtrs.size();
        keysPtrs.push_back(& key);
    }
}

void Dumper::ClearLine()
{
    m_ctx.line = Line();
    if (m_settings.isCArray)
    {
        m_ctx.line.offset = "//";
        m_ctx.line.ascii  = "// ";
    }
}

void Dumper::Generate(void* bufferVoid, const size_t bufferLength)
{
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    buffer = FindKeysAndShiftStartOfBuffer(bufferLength, buffer);

    utilities::PrintSeparator();

    GenerateImpl(buffer);

    utilities::PrintSeparator();

    utilities::PrintRange(m_settings.range, m_settings.shift, m_settings.showDetailed);

//    utilities::PrintKeysResults(m_keys, m_settings.showDetailed, m_settings.shift);

    for (auto& key : m_keysPtrs) //m_keys.hexKeysVector)
    {
        utilities::PrintFoundKeyResults(*key, m_settings.showDetailed, m_settings.shift);
    }
}

void Dumper::GenerateImpl(uint8_t* buffer)
{
//    std::string currentWord;
    IsVisible charIsVisible;

    if (m_settings.isCArray)
        std::cout << "{\n";

    if (PositionInLine(m_settings.range.begin) != 0)
        PrepareFirstLine(m_settings.range.begin);

    for (size_t index = m_settings.range.begin; index <= m_settings.range.end; ++index)
    {
        if (PositionInLine(index) == 0)
        {
            m_ctx.range.begin = index;
            m_ctx.line.offset.append(GetOffsetFromIndex(index));
        }

        charIsVisible = IsCharVisible(buffer, index);

        if (IsNextWordStart(index, charIsVisible) && m_settings.newLine)
            PrintLineAndIntend(index);

        utilities::Color currentColor = GetColor(index, buffer[index]);

        AppendCurrentDumpLine(buffer, index, currentColor);

        AppendCurrentAsciiLine(buffer, index, charIsVisible, currentColor);

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
    if (m_settings.isCArray)
        std::cout << "};\n";
}

uint8_t* Dumper::FindKeysAndShiftStartOfBuffer(const size_t bufferLength, uint8_t* buffer)
{
    if ((m_settings.range.end == 0 && m_settings.range.begin != 0) ||
        (m_settings.range.end >= bufferLength))
    {
        m_settings.range.end = bufferLength - 1;
    }

    if (!m_keys.hkeyFrom.value.empty() || !m_keys.hkeyTill.value.empty())
    {
        m_settings.range = finder::FindRangeForPairOfKeys(buffer, m_settings.range, m_keys.hkeyFrom, m_keys.hkeyTill);
        if (m_settings.countBytesAfterHkeyFrom != 0)
        {
            const size_t expectedEnd = m_settings.range.begin + m_settings.countBytesAfterHkeyFrom - 1;
            if (expectedEnd < m_settings.range.end)
                m_settings.range.end = expectedEnd;
        }
    }

    if (m_settings.useRelativeAddress)
        buffer = ShiftBeginOfBufferAndResults(buffer, m_settings);

    m_keys.key.results  = finder::FindIndexesForKey(buffer, m_settings.range, m_keys.key);
    std::cout << m_keys.key.value << " " << m_keys.key.results.size()  << "\n";

    AddKeyInVector(m_keys.hkeyFrom, m_keysPtrs);
    AddKeyInVector(m_keys.hkeyTill, m_keysPtrs);
    AddKeyInVector(m_keys.key,      m_keysPtrs);

    m_limitingKeysCount = m_keysPtrs.size();

    for (const auto& keyValue : m_settings.hkeyValueVector)
    {
        auto* key =  new TheKey(keyValue);
        key->id = m_keysPtrs.size();
        m_keysPtrs.push_back(key);
    }

    for (const auto& key : m_keysPtrs)
        std::cout << key->id << ", " << key->value << " " << key->results.size() << "\n";

    for (size_t index = m_limitingKeysCount; index < m_keysPtrs.size(); ++index)
    {
        auto& key = m_keysPtrs[index];
        key->results = finder::FindIndexesForHexKey(buffer, m_settings.range, *key);
    }

    return buffer;
}

utilities::Color Dumper::GetColor(size_t index, const uint8_t currentValue) const
{
    for (const auto* key : m_keysPtrs)
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

    // TODO scroll
    for (auto keyPosition : m_keys.key.results)
        if (IsLineNearKeys(range, keyPosition, m_keys.key.length))
            return false;

    for (auto keyPosition : m_keys.hkeyFrom.results)
        if (IsLineNearKeys(range, keyPosition, m_keys.hkeyFrom.length))
            return false;

    for (auto keyPosition : m_keys.hkeyTill.results)
        if (IsLineNearKeys(range, keyPosition, m_keys.hkeyTill.length))
            return false;

    return true;
}
bool Dumper::IsLineNearKeys(const Range& range, size_t position, size_t keySize) const
{
    return range.begin <= position  + m_settings.countBytesAfterKey + keySize - 1 &&
              position <= range.end + m_settings.countBytesBeforeKey;
}

uint8_t* Dumper::ShiftBeginOfBufferAndResults(uint8_t* buffer, DumperSettings& settings)
{
    if (!m_keys.hkeyFrom.results.empty())
    {
        const auto shift = m_keys.hkeyFrom.results.front();
        for (auto& index : m_keys.hkeyFrom.results)
            index -= shift;
        for (auto& index : m_keys.hkeyTill.results)
            index -= shift;
    }

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

// Actual for --shift=true
void Dumper::PrepareFirstLine(size_t index)
{
    m_ctx.line.offset += GetOffsetFromIndex(index);

    const auto positionInLine = PositionInLine(index);
    m_ctx.line.dump   += GetSpacesForBeginOfDumpLine(positionInLine);
    m_ctx.line.ascii  += GetSpacesForBeginOfAsciiLine(positionInLine);
}

void Dumper::AppendCurrentDumpLine(const uint8_t* buffer, const size_t index, utilities::Color currentColor)
{
    if (currentColor != utilities::Color::Normal)
        m_ctx.line.dump.append(utilities::ColorAnsiCode[currentColor]);

    m_ctx.line.dump.append(GetDumpValue(buffer[index]));

    if (currentColor != utilities::Color::Normal)
        m_ctx.line.dump.append(utilities::ColorAnsiCode[utilities::Color::Normal]);

    if (m_settings.isCArray && index < m_settings.range.end)
        m_ctx.line.dump.append(",");

    if (m_settings.isCArray && index == m_settings.range.end)
        m_ctx.line.dump.append(" ");

    m_ctx.line.dump.append(" ");

    if (IsPositionLastInColumn(index) && m_settings.isSpaceBetweenDumpBytes)
        m_ctx.line.dump.append(" ");

    if (index == m_settings.range.end)
        m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index)));
}

void Dumper::AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, utilities::Color currentColor)
{
    if (currentColor != utilities::Color::Normal)
        m_ctx.line.ascii.append(utilities::ColorAnsiCode[currentColor]);

    if (isVisible.current)
    {
        m_ctx.line.ascii += static_cast<char>(buffer[index]);
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
            if (buffer[index] == 0)
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

    m_ctx.line.offset = GetOffsetFromIndex(index);
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

std::string Dumper::GetDumpValue(uint8_t value) const
{
    std::string dumpValueStr;

    if (m_settings.isCArray)
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

    if (m_settings.isCArray)
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

    if (m_settings.isCArray)
        lengthOfRestLine += dump::size::AdditionalCharsForCArray * restBytesInLine;

    std::string spaces(lengthOfRestLine, ' ');

    return spaces;
}


std::string Dumper::GetOffsetFromIndex(size_t i) const
{
    std::string result;

    if (m_settings.offset == OffsetTypes::Dec || m_settings.offset == OffsetTypes::Both)
    {
        if (m_settings.shift)
            result += (boost::format("%5d") % i).str();

        if (m_settings.shift && m_settings.showDetailed)
            result += "/";

        if (m_settings.showDetailed || !m_settings.shift)
            result += (boost::format("%5d") % (m_settings.shift + i)).str();
    }

    if (m_settings.offset == OffsetTypes::Both)
        result += "'";

    if (m_settings.offset == OffsetTypes::Hex || m_settings.offset == OffsetTypes::Both)
    {
        if (m_settings.shift)
            result += (boost::format("0x%04x") % (i)).str();

        if (m_settings.shift && m_settings.showDetailed)
            result += "/";

        if (m_settings.showDetailed || !m_settings.shift)
            result += (boost::format("0x%04x") % (m_settings.shift + i)).str();
    }

    if (m_settings.offset != OffsetTypes::None)
        result += ": ";

    return result;
}

void Ctx::Print(bool isOffsetVisible, bool isDumpVisible, bool isAsciiVisible, bool isCArray, bool isDebugVisible)
{
    if (isOffsetVisible && !isCArray)
        std::cout << line.offset;

    if (isDumpVisible)
        std::cout << line.dump;

    if (isOffsetVisible && isCArray)
        std::cout << line.offset;

    if (isAsciiVisible)
        std::cout << line.ascii;

    if (isDebugVisible)
        std::cout << "\t" << line.debug;

    std::cout << std::endl;
}

void Dumper::PrintAndClearLine(const bool isNewGroupAfterSkippedLines)
{
    if (isNewGroupAfterSkippedLines)
        utilities::PrintEmptyLine(m_settings.isShowEmptyLines, m_ctx.skipLinesCount);

    m_ctx.line.debug = "<" + std::to_string(m_ctx.range.begin) + "..." + std::to_string(m_ctx.range.end) + "> ";

    m_ctx.Print(m_settings.isShowOffset,
            m_settings.isShowDump,
            m_settings.isShowAscii,
            m_settings.isCArray,
            m_settings.isShowDebug);

    ClearLine();
}

} // namespace dump
