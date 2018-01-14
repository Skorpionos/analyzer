#include "dump.h"

#include <boost/format.hpp>

namespace dump
{

Dumper::Dumper(DumperSettings settings)
{
    SetSettings(settings);
}

void Dumper::SetSettings(dump::DumperSettings settings)
{
    m_settings = settings;
}

void Dumper::Print(void* bufferVoid, size_t length)
{
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    std::string currentWord;

    bool previousCharIsVisible = false;

    for (size_t index = 0; index < length; ++index)
    {
        if (PositionInLine(index) == 0)
            m_ctx.line.offset = GetOffsetFromIndex(index);

        const bool currentCharIsVisible = isprint(buffer[index]);

        // "Do enter"
        const auto isNextWord = !previousCharIsVisible && currentCharIsVisible && index != 0;
        if (isNextWord && m_settings.newline)
        {
            m_ctx.line.dump.append(GetSpacesForRestOfLine(PositionInLine(index-1)));
            m_ctx.Print();
            m_ctx.Clear();

            m_ctx.line.offset = GetOffsetFromIndex(index);
            m_ctx.line.dump.append(GetSpacesForBeginOfLine(PositionInLine(index)));
        }

        const auto currentDumpValue = boost::format("%02x ") % static_cast<uint16_t>(buffer[index]);
        m_ctx.line.dump.append(currentDumpValue.str());

        if (currentCharIsVisible)
        {
            m_ctx.line.ascii += buffer[index];
            currentWord += buffer[index];
        }
        else
        {
            m_ctx.line.ascii += m_settings.placeHolder;
        }

        if (isPositionLastInColumn(index))
            m_ctx.line.dump.append(" ");

        if (isPositionLastInColumn(index) && m_settings.isSpaceBetweenBytes)
            m_ctx.line.ascii.append(" ");

        if (index == length - 1)
            m_ctx.line.dump.append(GetSpacesForRestOfLine(PositionInLine(index)));

        const auto endOfCurrentLine = (PositionInLine(index) == m_settings.bytesInLine - 1) || (index == length - 1);
        if (endOfCurrentLine)
        {
            m_ctx.Print();
            m_ctx.Clear();
        }
        previousCharIsVisible = currentCharIsVisible;
    }

}

size_t Dumper::PositionInLine(size_t index) const
{
    return index % m_settings.bytesInLine;
}

bool Dumper::isPositionLastInColumn(size_t index) const
{
    return index % 8 == 7;
}

std::string Dumper::GetSpacesForBeginOfLine(const size_t positionInLine) const
{
    if (positionInLine == 0)
        return "";

    const size_t lengthOfBeginOfLine = 3 * positionInLine + (positionInLine) / 8;

    std::string spaces(lengthOfBeginOfLine, ' ');
//    std::string debugInfo = "{begin=" + std::to_string(lengthOfBeginOfLine) + "}";
    //    m_ctx.line.debug.append(debugInfo);
    return spaces;
}

std::string Dumper::GetSpacesForRestOfLine(const size_t positionInLine) const
{
    const auto restBytesInLine = m_settings.bytesInLine - positionInLine - 1;
    size_t lengthOfRestLine = 3 * (restBytesInLine) + (restBytesInLine + 7) / 8;

    std::string spaces(lengthOfRestLine, ' ');
//    std::string debugInfo = "{rest=" + std::to_string(lengthOfRestLine) + "}";
//    m_ctx.line.debug.append(debugInfo);
    return spaces;
}


Offset GetOffsetType(const std::string& offset)
{
    if (offset == "hex")
        return Offset::Hex;
    if (offset == "dec")
        return Offset::Dec;
    if (offset == "both")
        return Offset::Both;
    if (offset == "none" || offset == "false")
        return Offset::None;

    return Offset::Unknown;
}

std::string Dumper::GetOffsetFromIndex(size_t i) const
{
    std::string result;
    switch (m_settings.offset)
    {
        case (Offset::Dec):
            return (boost::format("%5d: ") % i).str();

        case (Offset::Hex):
            return (boost::format("0x%04x: ") % i).str();

        case (Offset::Both):
            return (boost::format("%5d'0x%04x: ") % i % i).str();

        case Offset::None:
        case Offset::Unknown:
            return "";
    }
}

size_t Dumper::OffsetLength()
{
    switch (m_settings.offset)
    {
        case (Offset::Dec):
            return 8;
        case (Offset::Hex):
            return 9;
        case (Offset::Both):
            return 15;
        case Offset::None:
        case Offset::Unknown:
            return 0;
    }
}

size_t Dumper::HexDumpLength() const
{
    if (m_settings.isShowDump)
        return m_settings.columnCount * 25;
    else
        return 0;
}

} // namespace dump
