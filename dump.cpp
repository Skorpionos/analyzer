#include "dump.h"

namespace dump
{

Dumper::Dumper(DumperSettings settings)
{
    SetSettings(settings);
    ClearLine();
}

void Dumper::SetSettings(dump::DumperSettings settings)
{
    m_settings = settings;
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
        FindByHexKey(buffer, bufferLength,  m_settings.hkey, m_hkeyResultPositions, m_countBytesInHexKey);

    if (m_settings.useRelativeAddress)
        buffer = ShiftStartOffset(buffer);

    size_t fromPosition;
    size_t tillPosition;
    if (!m_settings.from.empty() || !m_settings.till.empty())
    {
        FindRangeForPairOfKeys(buffer, bufferLength, m_settings.from, m_settings.till, fromPosition, tillPosition);

        m_settings.begin = fromPosition;
        m_settings.end = tillPosition;
    }

    PrintSeparator();

    GenerateImpl(buffer);

    PrintSeparator();

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
        if (PositionInLine(index) == 0)
            m_ctx.line.offset.append(GetOffsetFromIndex(index));

        charIsVisible = IsCharVisible(buffer, index);

        if (IsNextWordStart(index, charIsVisible) && m_settings.newLine)
            PrintLineAndIntend(index);

        Color currentColor = GetColor(index);

        AppendCurrentDumpLine(buffer, index, currentColor);

        AppendCurrentAsciiLine(buffer, index, charIsVisible, currentColor);

        if (IsEndOfCurrentLine(index))
        {
            CompleteCurrentDumpLine();
            PrintAndClearLine();
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

uint8_t* Dumper::ShiftStartOffset(uint8_t* buffer)
{
    m_settings.startOffset = m_settings.begin;
    m_settings.begin = 0;
    m_settings.end -= m_settings.startOffset;

    buffer += m_settings.startOffset;
    return buffer;
}

void Dumper::PrintFoundKeyResults(const std::vector<size_t>& positionResults, std::string& keyName) const
{
    if (keyName == "")
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
        m_ctx.line.dump.append(setColor[currentColor]);

    m_ctx.line.dump.append(GetDumpValue(buffer[index]));

    if (currentColor != Color::Normal)
        m_ctx.line.dump.append(setColor[Color::Normal]);

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
        m_ctx.line.ascii.append(setColor[currentColor]);

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
        m_ctx.line.ascii.append(setColor[Color::Normal]);
}

void Dumper::PrintLineAndIntend(size_t index)
{
    m_ctx.line.dump.append(GetSpacesForRestOfDumpLine(PositionInLine(index - 1)));
    if (!m_settings.ladder)
        m_ctx.line.dump.append(m_ctx.line.indent);
    PrintAndClearLine();

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
    isVisible.previous = index > 0 && isprint(buffer[index - 1]);
    isVisible.current = isprint(buffer[index]);
    isVisible.next = index < m_settings.end && isprint(buffer[index + 1]);

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

    size_t lengthOfBeginOfLine = CharCountPerByte * positionInLine;

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

    size_t lengthOfRestLine = CharCountPerByte * restBytesInLine;
    if (m_settings.isSpaceBetweenDumpBytes)
        lengthOfRestLine += (restBytesInLine + m_settings.bytesInGroup - 1) / m_settings.bytesInGroup;

    if (m_settings.IsCArray)
        lengthOfRestLine += dump::AdditionalCharCountForCArray * restBytesInLine;

    std::string spaces(lengthOfRestLine, ' ');

    return spaces;
}

Offset GetOffsetType(const std::string& offset)
{
    if (offset == "hex")
        return Offset::Hex;
    if (offset == "dec")
        return Offset::Dec;
    if (offset == "both" || offset == "true")
        return Offset::Both;
    if (offset == "none" || offset == "false")
        return Offset::None;

    return Offset::Unknown;
}

std::string Dumper::GetOffsetFromIndex(size_t i) const
{
    std::string result;

    if (m_settings.offset == Offset::Dec || m_settings.offset == Offset::Both)
    {
        if (m_settings.startOffset)
            result += (boost::format("%5d") % i).str();

        if (m_settings.startOffset && m_settings.detailedOffset)
            result += "|";

        if (m_settings.detailedOffset || !m_settings.startOffset)
            result += (boost::format("%5d") % (m_settings.startOffset + i)).str();
    }

    if (m_settings.offset == Offset::Both)
        result += "'";

    if (m_settings.offset == Offset::Hex || m_settings.offset == Offset::Both)
    {
        if (m_settings.startOffset)
            result += (boost::format("0x%04x") % (i)).str();

        if (m_settings.startOffset && m_settings.detailedOffset)
            result += "|";

        if (m_settings.detailedOffset || !m_settings.startOffset)
            result += (boost::format("0x%04x") % (m_settings.startOffset + i)).str();
    }

    if (m_settings.offset != Offset::None)
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

void Dumper::PrintAndClearLine()
{
    m_ctx.Print(m_settings.isShowOffset,
                m_settings.isShowDump,
                m_settings.isShowAscii,
                m_settings.IsCArray,
                m_settings.isShowDebug);
    ClearLine();
}

bool Dumper::FindIndexForKey(const uint8_t* buffer, size_t bufferLength, std::string key,
        std::vector<size_t>& resultPositions)
{
    return FindIndexForKeyInBytes(buffer, bufferLength, reinterpret_cast<const uint8_t*>(m_settings.key.c_str()),
            m_settings.key.length(), resultPositions);
}

bool Dumper::FindIndexForKeyInBytes(const uint8_t* buffer, size_t bufferLength, const uint8_t* key, size_t keyLength,
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

bool Dumper::FindRangeForPairOfKeys(uint8_t* buffer, size_t bufferLength, std::string hexKeyFrom,
        std::string hexKeyTill,
        size_t& fromPosition, size_t& tillPosition)
{
    std::vector<size_t> resultsFrom;
    std::vector<size_t> resultsTill;

    size_t countBytesInKeyFrom;
    size_t countBytesInKeyTill;

    FindByHexKey(buffer, bufferLength, hexKeyFrom, resultsFrom, countBytesInKeyFrom);
    FindByHexKey(buffer, bufferLength, hexKeyTill, resultsTill, countBytesInKeyTill);

    PrintFoundKeyResults(resultsFrom, m_settings.from);
    PrintFoundKeyResults(resultsTill, m_settings.till);

    fromPosition = !resultsFrom.empty() ? resultsFrom.front() : 0;
    tillPosition = !resultsTill.empty() ? resultsTill.back() + countBytesInKeyTill - 1: 0;

    return !resultsFrom.empty() || !resultsTill.empty();
}

void Dumper::FindByHexKey(const uint8_t* buffer, size_t bufferLength, std::string& hexKey,
                          std::vector<size_t>& foundPositions, size_t& countBytesInKey)
{
    std::vector<std::string> words;

    split(words, hexKey, boost::algorithm::is_any_of(" "));
    countBytesInKey = words.size();

    uint8_t* hexKeyBytes = new uint8_t[countBytesInKey];

    GetHexKeyBytes(words, hexKeyBytes);
    FindIndexForKeyInBytes(buffer, bufferLength, hexKeyBytes, words.size(), foundPositions);
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

} // namespace dump
