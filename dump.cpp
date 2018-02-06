#include "dump.h"
#include "finder.h"

namespace dump
{

Dumper::Dumper(DumperSettings settings)
{
    SetSettings(std::move(settings));
    ClearLine();
}

void Dumper::SetSettings(dump::DumperSettings settings)
{
    m_settings = std::move(settings);
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

    utilities::PrintKeysResults(m_settings.keyFrom, m_settings.keyTill, m_settings.key, m_settings.hkey,
                                m_settings.shift, m_settings.showDetailed, m_foundKeys.from.vector,
                                m_foundKeys.till.vector, m_foundKeys.key.vector, m_foundKeys.hkey.vector);
}

uint8_t* Dumper::FindKeysAndShiftStartOfBuffer(const size_t bufferLength, uint8_t* buffer)
{
    if ((m_settings.range.end == 0 && m_settings.range.begin != 0) ||
        (m_settings.range.end >= bufferLength))
    {
        m_settings.range.end = bufferLength - 1;
    }

    if (!m_settings.keyFrom.empty() || !m_settings.keyTill.empty())
    {
        m_settings.range = finder::FindRangeForPairOfKeys(buffer, m_settings.range, m_settings.keyFrom,
                                                          m_settings.keyTill, m_foundKeys.from, m_foundKeys.till);
        if (m_settings.countBytesAfterHkeyFrom != 0)
        {
            const size_t expectedEnd = m_settings.range.begin + m_settings.countBytesAfterHkeyFrom - 1;
            if (expectedEnd < m_settings.range.end)
                m_settings.range.end = expectedEnd;
        }
    }

    if (m_settings.useRelativeAddress)
        buffer = ShiftBeginOfBufferAndResults(buffer, m_settings);

    m_foundKeys.key.vector  = finder::FindIndexesForKey   (buffer, m_settings.range, m_settings.key);
    m_foundKeys.hkey.vector = finder::FindIndexesForHexKey(buffer, m_settings.range, m_settings.hkey, m_foundKeys.hkey.length);
    return buffer;
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

        Color currentColor = GetColor(index, buffer[index]);

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

Color Dumper::GetColor(size_t index, const uint8_t currentValue) const
{
    if (!m_foundKeys.key.vector.empty())
    {
        for (auto position : m_foundKeys.key.vector)
            if (index - position < m_settings.key.size())
                return Color::Red;
    }

    if (!m_foundKeys.hkey.vector.empty())
    {
        for (auto position : m_foundKeys.hkey.vector)
            if (index - position < m_foundKeys.hkey.length)
                return Color::Green;
    }

    if (!m_foundKeys.from.vector.empty())
    {
        for (auto position : m_foundKeys.from.vector)
            if (index - position < m_foundKeys.from.length)
                return Color::Blue;
    }

    if (!m_foundKeys.till.vector.empty())
    {
        for (auto position : m_foundKeys.till.vector)
            if (index - position < m_foundKeys.till.length)
                return Color::Yellow;
    }
    if (currentValue == 0 && m_settings.printZeroAsGrey)
        return Color::Grey;

    return Color::Normal;
}

bool Dumper::IsLineSkipped(Range range)
{
    if (!m_settings.skipTextWithoutKeys)
        return false;

    for (auto keyPosition : m_foundKeys.key.vector)
        if (IsLineNearKeys(range, keyPosition, m_settings.key.size()))
            return false;

    for (auto keyPosition : m_foundKeys.hkey.vector)
        if (IsLineNearKeys(range, keyPosition, m_foundKeys.hkey.length))
            return false;

    for (auto keyPosition : m_foundKeys.from.vector)
        if (IsLineNearKeys(range, keyPosition, m_foundKeys.from.length))
            return false;

    for (auto keyPosition : m_foundKeys.till.vector)
        if (IsLineNearKeys(range, keyPosition, m_foundKeys.till.length))
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
    if (!m_foundKeys.from.vector.empty())
    {
        const auto shift = m_foundKeys.from.vector.front();
        for (auto& index : m_foundKeys.from.vector)
            index -= shift;
        for (auto& index : m_foundKeys.till.vector)
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

void Dumper::AppendCurrentDumpLine(const uint8_t* buffer, const size_t index, Color currentColor)
{
    if (currentColor != Color::Normal)
        m_ctx.line.dump.append(utilities::CololAnsiCode[currentColor]);

    m_ctx.line.dump.append(GetDumpValue(buffer[index]));

    if (currentColor != Color::Normal)
        m_ctx.line.dump.append(utilities::CololAnsiCode[Color::Normal]);

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

void Dumper::AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, Color currentColor)
{
    if (currentColor != Color::Normal)
        m_ctx.line.ascii.append(utilities::CololAnsiCode[currentColor]);

    if (isVisible.current)
    {
        m_ctx.line.ascii += buffer[index];
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

    if (currentColor != Color::Normal)
        m_ctx.line.ascii.append(utilities::CololAnsiCode[Color::Normal]);
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

OffsetTypes GetOffsetType(const std::string& offset)
{
    if (offset == "hex")
        return OffsetTypes::Hex;
    if (offset == "dec")
        return OffsetTypes::Dec;
    if (offset == "both" || offset == "true")
        return OffsetTypes::Both;
    if (offset == "none" || offset == "false")
        return OffsetTypes::None;

    return OffsetTypes::Unknown;
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
