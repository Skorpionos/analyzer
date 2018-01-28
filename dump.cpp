#include "dump.h"

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
    if (m_settings.IsCArray)
    {
        m_ctx.line.offset = "//";
        m_ctx.line.ascii  = "// ";
    }
}

void Dumper::Generate(void* bufferVoid, size_t bufferLength)
{
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    if (m_settings.end == 0 || m_settings.end >= bufferLength)
        m_settings.end = bufferLength - 1;

    if (!m_settings.key.empty())
        FindIndexForKey(buffer, bufferLength, m_settings.key, m_keyResultPositions);
    
    if (!m_settings.hkey.empty())
        FindIndexForHexKey(buffer, bufferLength, m_settings.hkey, m_hkeyResultPositions, m_countBytesInHexKey);

    if (m_settings.useRelativeAddress)
        buffer = ShiftStartOffset(buffer);

    Range range;
    if (!m_settings.from.empty() || !m_settings.till.empty())
    {
        FindRangeForPairOfKeys(buffer, bufferLength, m_settings.from, m_settings.till, range);

        m_settings.begin = range.begin;
        m_settings.end = range.end;
    }

    utilities::PrintSeparator();

    GenerateImpl(buffer);

    utilities::PrintSeparator();

    std::cout << "address <" << m_settings.begin << "..." << m_settings.end << ">";
    std::cout << ", size:" << m_settings.end - m_settings.begin + 1 << "\n";

    PrintFoundKeyResults(m_keyResultPositions, m_settings.key);
    PrintFoundKeyResults(m_hkeyResultPositions, m_settings.hkey);
}

void Dumper::GenerateImpl(uint8_t* buffer)
{
//    std::string currentWord;

    IsVisible charIsVisible;

    if (m_settings.IsCArray)
        std::cout << "{\n";

    if (PositionInLine(m_settings.begin) != 0)
        PrepareFirstLine();

    for (size_t index = m_settings.begin; index <= m_settings.end; ++index)
    {
        if (m_ctx.line.isEmpty)
        {
            m_ctx.line.isEmpty = false;
            m_ctx.range.begin = index;
        }

        if (PositionInLine(index) == 0)
            m_ctx.line.offset.append(GetOffsetFromIndex(index));

        charIsVisible = IsCharVisible(buffer, index);

        if (IsNextWordStart(index, charIsVisible) && m_settings.newLine)
            PrintLineAndIntend(index);

        Color currentColor = GetColor(index);

        AppendCurrentDumpLine(buffer, index, currentColor);

        AppendCurrentAsciiLine(buffer, index, charIsVisible, currentColor);

        m_ctx.range.end = index;

        if (IsEndOfCurrentLine(index))
        {
            CompleteCurrentDumpLine();

            bool lineIsSkip = IsSkipLine(m_ctx.range);

            PrintAndClearLine(lineIsSkip, m_ctx.isPreviousSkip);

            m_ctx.isPreviousSkip = lineIsSkip;
        }
    }
    if (m_settings.IsCArray)
        std::cout << "};\n";
}

Color Dumper::GetColor(size_t index) const
{
    if (!m_settings.key.empty())
    {
        for (auto position : m_keyResultPositions)
            if (index - position < m_settings.key.size())
                return Color::Red;
    }

    if (!m_settings.hkey.empty())
    {
        for (auto position : m_hkeyResultPositions)
            if (index - position < m_countBytesInHexKey)
                return Color::Green;
    }

    return Color::Normal;
}

bool Dumper::IsSkipLine(Range range)
{
    if (!m_settings.skipLines)
        return false;

    for (auto keyPosition : m_keyResultPositions)
        if (IsLineNearKeys(range, keyPosition, m_settings.key.size()))
            return false;

    for (auto keyPosition : m_hkeyResultPositions)
        if (IsLineNearKeys(range, keyPosition, m_countBytesInHexKey))
            return false;

    return true;
}
bool Dumper::IsLineNearKeys(const Range& range, size_t position, size_t keySize) const
{
    return range.begin <= position  + m_settings.countBytesAfterKey + keySize - 1 &&
              position <= range.end + m_settings.countBytesBeforeKey;
}

uint8_t* Dumper::ShiftStartOffset(uint8_t* buffer)
{
    m_settings.startOffset = m_settings.begin;
    m_settings.begin = 0;
    m_settings.end -= m_settings.startOffset;

    buffer += m_settings.startOffset;
    return buffer;
}

void Dumper::CompleteCurrentDumpLine()
{
    if (!m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
}

void Dumper::PrepareFirstLine()
{
    // Actual for --fromStart=true
    m_ctx.line.offset += GetOffsetFromIndex(m_settings.begin);
    m_ctx.line.dump   += GetSpacesForBeginOfDumpLine(PositionInLine(m_settings.begin));
    m_ctx.line.ascii  += GetSpacesForBeginOfAsciiLine(PositionInLine(m_settings.begin));
}

void Dumper::AppendCurrentDumpLine(const uint8_t* buffer, const size_t index, Color currentColor)
{
    if (currentColor != Color::Normal)
        m_ctx.line.dump.append(colorString[currentColor]);

    m_ctx.line.dump.append(GetDumpValue(buffer[index]));

    if (currentColor != Color::Normal)
        m_ctx.line.dump.append(colorString[Color::Normal]);

    if (m_settings.IsCArray && index < m_settings.end)
        m_ctx.line.dump.append(",");

    if (m_settings.IsCArray && index == m_settings.end)
        m_ctx.line.dump.append(" ");

    m_ctx.line.dump.append(" ");

    if (IsPositionLastInColumn(index) && m_settings.isSpaceBetweenDumpBytes)
        m_ctx.line.dump.append(" ");

    if (index == m_settings.end)
        m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index)));
}

void Dumper::AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, Color currentColor)
{
    if (currentColor != Color::Normal)
        m_ctx.line.ascii.append(colorString[currentColor]);

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
        m_ctx.line.ascii.append(colorString[Color::Normal]);
}

void Dumper::PrintLineAndIntend(size_t index)
{
    m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index - 1)));
    if (!m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
    PrintAndClearLine(false, false);

    m_ctx.line.offset = GetOffsetFromIndex(index);
    m_ctx.line.indent = GetSpacesForBeginOfDumpLine(PositionInLine(index));
    if (m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
}

bool Dumper::IsEndOfCurrentLine(size_t index) const
{
    return (PositionInLine(index) == m_settings.bytesInLine - 1) || (index == m_settings.end);
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
    isVisible.next        = index < m_settings.end && isprint(buffer[index + 1]);

    return isVisible;
}

std::string Dumper::GetDumpValue(uint8_t value) const
{
    std::string dumpValueStr;
    if (m_settings.IsCArray)
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

    size_t lengthOfBeginOfLine = dump::CharCountPerByte * positionInLine;

    if (m_settings.isSpaceBetweenDumpBytes)
        lengthOfBeginOfLine += positionInLine / m_settings.bytesInGroup;

    if (m_settings.IsCArray)
        lengthOfBeginOfLine += dump::AdditionalCharCountForCArray * positionInLine;

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

    if (m_settings.IsCArray)
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
        if (m_settings.startOffset)
            result += (boost::format("%5d") % i).str();

        if (m_settings.startOffset && m_settings.detailedOffset)
            result += "|";

        if (m_settings.detailedOffset || !m_settings.startOffset)
            result += (boost::format("%5d") % (m_settings.startOffset + i)).str();
    }

    if (m_settings.offset == OffsetTypes::Both)
        result += "'";

    if (m_settings.offset == OffsetTypes::Hex || m_settings.offset == OffsetTypes::Both)
    {
        if (m_settings.startOffset)
            result += (boost::format("0x%04x") % (i)).str();

        if (m_settings.startOffset && m_settings.detailedOffset)
            result += "|";

        if (m_settings.detailedOffset || !m_settings.startOffset)
            result += (boost::format("0x%04x") % (m_settings.startOffset + i)).str();
    }

    if (m_settings.offset != OffsetTypes::None)
        result += ": ";

//    if (m_settings.IsCArray)
//        result += "*/  ";

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

void Dumper::PrintAndClearLine(const bool isSkip, const bool isPreviousSkip)
{
    if (isSkip)
    {
        if (!isPreviousSkip)
            utilities::PrintSkipLine();
    }
    else
    {
        m_ctx.line.debug = "<" + std::to_string(m_ctx.range.begin) + "..." + std::to_string(m_ctx.range.end) + "> ";

        m_ctx.Print(m_settings.isShowOffset,
                m_settings.isShowDump,
                m_settings.isShowAscii,
                m_settings.IsCArray,
                m_settings.isShowDebug);
    }
    ClearLine();
}


// TODO move find functions to separate class
bool Dumper::FindIndexForKey(const uint8_t* buffer, size_t bufferLength, std::string key,
        std::vector<size_t>& resultPositions)
{
    return FindIndexForBytesKey(buffer, bufferLength, reinterpret_cast<const uint8_t*>(key.c_str()),
            key.length(), resultPositions);
}

// TODO use standard library if possible
bool Dumper::FindIndexForBytesKey(const uint8_t* buffer, size_t bufferLength, const uint8_t* key, size_t keyLength,
        std::vector<size_t>& resultPositions)
{
    bool result = false;
    size_t bufferIndex = 0;
    size_t previousBufferIndex = 0;
    while (bufferIndex < bufferLength)
    {
        size_t keyIndex = 0;
        size_t foundStringLength = 0;
        size_t indexFirstLetterOfKey = bufferIndex;
        while (keyIndex < keyLength && bufferIndex < bufferLength && buffer[bufferIndex] == key[keyIndex])
        {
            ++foundStringLength;
            ++bufferIndex;
            ++keyIndex;
        }
        if (foundStringLength == keyLength)
        {
            result = true;
            resultPositions.push_back(indexFirstLetterOfKey);
        }
        if (bufferIndex == previousBufferIndex)
            ++bufferIndex;

        previousBufferIndex = bufferIndex;
    }
    return result;
}

void Dumper::FindIndexForHexKey(const uint8_t* buffer, size_t bufferLength, std::string& hexKey,
        std::vector<size_t>& foundPositions, size_t& countBytesInKey)
{
    std::vector<std::string> words;

    split(words, hexKey, boost::algorithm::is_any_of(" "));
    countBytesInKey = words.size();

    uint8_t* hexKeyBytes = new uint8_t[countBytesInKey];

    GetHexKeyBytes(words, hexKeyBytes);
    FindIndexForBytesKey(buffer, bufferLength, hexKeyBytes, words.size(), foundPositions);
}

void Dumper::GetHexKeyBytes(const std::vector<std::string>& words, uint8_t* hexKeyBytes) const
{
    for (int i = 0; i < words.size(); ++i)
    {
        std::stringstream stream;
        stream << std::hex << words[i];

        uint16_t byte;
        stream >> byte;

        hexKeyBytes[i] = static_cast<uint8_t>(byte);
    }
}

bool Dumper::FindRangeForPairOfKeys(uint8_t* buffer, size_t bufferLength,
        std::string hexKeyFrom, std::string hexKeyTill,
        Range& range)
{
    std::vector<size_t> resultsFrom;
    std::vector<size_t> resultsTill;

    size_t countBytesInKeyFrom;
    size_t countBytesInKeyTill;

    FindIndexForHexKey(buffer, bufferLength, hexKeyFrom, resultsFrom, countBytesInKeyFrom);
    FindIndexForHexKey(buffer, bufferLength, hexKeyTill, resultsTill, countBytesInKeyTill);

    PrintFoundKeyResults(resultsFrom, m_settings.from);
    PrintFoundKeyResults(resultsTill, m_settings.till);

    range.begin = !resultsFrom.empty() ? resultsFrom.front() : 0;
    range.end   = !resultsTill.empty() ? resultsTill.back() + countBytesInKeyTill - 1: 0;

    return !resultsFrom.empty() || !resultsTill.empty();
}

void Dumper::PrintFoundKeyResults(const std::vector<size_t>& positionResults, std::string& keyName) const
{
    if (keyName.empty())
        return;

    std::cout << "for key '" << keyName << "' found " << positionResults.size() << " results: ";
    for (const size_t& position : positionResults)
    {
        int64_t relativePosition = position - m_settings.startOffset;
        std::cout << relativePosition;
        if (m_settings.startOffset != 0)
            std::cout << "/" << position;
        std::cout << " ";
    }
    std::cout << "\n";
}

} // namespace dump
