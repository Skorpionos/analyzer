#include "dump.h"
#include "utilities.h"

#include <boost/format.hpp>

namespace hexdump
{

using std::cout;
using std::endl;

Dumper::Dumper(DumperSettings settings)
{
    SetSettings(settings);
}

void Dumper::SetSettings(hexdump::DumperSettings settings)
{
    m_settings = settings;
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

void Dumper::Print(void* bufferVoid, size_t length)
{


    auto buffer = static_cast<uint8_t*>(bufferVoid);

    m_ctx.bytesInLine = 8 * m_settings.columnCount;

    std::string currentWord;
    bool previousIsVisible = false;
    for (size_t index = 0; index < length; ++index)
    {
        const auto currentPositionInLine = index % m_ctx.bytesInLine;
        if (currentPositionInLine == 0)
            PrintOffset(index);

        const auto currentDumpValue = boost::format("%02x ") % static_cast<uint16_t>(buffer[index]);
        m_ctx.line.dump.append(currentDumpValue.str());

        const bool currentCharIsVisible = isprint(buffer[index]);
        if (currentCharIsVisible)
        {
            m_ctx.line.ascii += buffer[index];
            currentWord += buffer[index];
        }
        else
        {
            m_ctx.line.ascii += m_settings.placeHolder;
        }
        //        if (!previousIsVisible && currentCharIsVisible)
        //        {
        //            cout << endl;
        //        }

        if (index % 8 == 7)
        {
            m_ctx.line.dump.append(" ");
            if (m_settings.isSpaceBetweenBytes)
                m_ctx.line.ascii.append(" ");
        }

        if (index == length - 1)
        {

            const auto restBytesInLine = m_ctx.bytesInLine - currentPositionInLine - 1;

            const size_t lengthOfRestLine = 3 * restBytesInLine + restBytesInLine / 8 + 1;

            std::string spaces(lengthOfRestLine, ' ');
            m_ctx.line.dump.append(spaces);
        }

        const auto endOfCurrentLine = (currentPositionInLine == m_ctx.bytesInLine - 1) || (index == length - 1);
        if (endOfCurrentLine)
        {
            cout << m_ctx.line.dump;
            cout << m_ctx.line.ascii << endl;

            m_ctx.line = hexdump::Line(); // TODO clear()
        }
        previousIsVisible = currentCharIsVisible;
    }


}

void Dumper::PrintOffset(size_t i) const
{
    switch (m_settings.offset)
    {
        case (Offset::Dec):cout << boost::format("%5d: ") % i;
            return;
        case (Offset::Hex):cout << boost::format("0x%04x: ") % i;
            return;
        case (Offset::Both):cout << boost::format("%5d'0x%04x: ") % i % i;
            return;
        case Offset::None:
        case Offset::Unknown:return;
    }
}

void Dumper::PrintArray(void* bufferVoid, size_t length)
{
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    cout << std::hex;

    for (size_t i = 0; i < length; ++i)
    {
        if (i > 0)
        {
            cout << ",";
            if (m_settings.isSpaceBetweenBytes)
                cout << " ";
        }
        if (i % m_settings.columnCount == 0 && i > 0)
            cout << endl;
        cout << boost::format("0x%02x") % static_cast<uint16_t>(buffer[i]);
    }
    std::cout << std::endl;
}

void Dumper::PrintOnlyVisible(void* bufferVoid, size_t length)
{
    std::cout << "mode: print only visible:" << std::endl;
    auto buffer = static_cast<uint8_t*>(bufferVoid);

    bool isFirstVisible = true;
    for (size_t i = 0; i < length; ++i)
    {
        if (isprint(buffer[i]))
        {
            cout << buffer[i];
            if (isFirstVisible && m_settings.newline)
            {
                isFirstVisible = false;
                cout << endl;
            }
        }
        else
        {
            isFirstVisible = true;
        }
    }
    std::cout << std::endl;
}

void Dumper::PrintCompressInvisible(void* bufferVoid, size_t length)
{
    std::cout << "mode: keep only one invisible:" << std::endl;

    auto buffer = static_cast<uint8_t*>(bufferVoid);

    cout << std::hex;

    for (size_t i = 0; i < length; ++i)
    {
        if (!isprint(buffer[i]))
        {
            if (m_settings.newline)
                cout << endl;
            else
                cout << " ";
            while (++i && i < length && !isprint(buffer[i]));
            if (i >= length)
                return;
        }
        cout << buffer[i];
    }
    std::cout << std::endl;
}

void Dumper::PrintRemoveFirstInvisible(void* bufferVoid, size_t length)
{
    std::cout << "mode: remove first invisible:" << std::endl;

    auto buffer = static_cast<uint8_t*>(bufferVoid);

    cout << std::hex;

    std::string str;
    bool isAlreadyDeleted = false;
    bool isFirstVisible = true;
    bool isPrevVisible = false;
    for (size_t i = 0; i < length; ++i)
    {
        bool currentVisible = isprint(buffer[i]);
        if (isPrevVisible && !currentVisible)
        {
            if (str.length() >= m_settings.length)
            {
                cout << str;
                if (m_settings.length != 0)
                    cout << endl;
            }

            str = "";
        }
        if (!currentVisible)
        {
            if (!isAlreadyDeleted)
            {
                isAlreadyDeleted = true;
                ++i;
                if (i >= length)
                    return;
            }
            isFirstVisible = true;
            if (m_settings.length == 0)
                cout << m_settings.placeHolder;
        }

        if (currentVisible)
        {
            if (isFirstVisible && m_settings.newline)
            {
                isFirstVisible = false;
                if (m_settings.length == 0)
                    cout << endl;
            }
            isAlreadyDeleted = false;
            str += buffer[i];
        }
        isPrevVisible = currentVisible;
    }
    std::cout << std::endl;
}

size_t Dumper::OffsetLength()
{
    switch (m_settings.offset)
    {
        case (Offset::Dec):return 8;
        case (Offset::Hex):return 9;
        case (Offset::Both):return 15;
        case Offset::None:
        case Offset::Unknown:return 0;
    }
}

size_t Dumper::HexDumpLength() const
{
    if (m_settings.isShowDump)
        return m_settings.columnCount * 25;
    else
        return 0;
}

} // namespace hexdump
