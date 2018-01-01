#include "dump.h"

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
    PrintSeparator();

    auto buffer = static_cast<uint8_t*>(bufferVoid);

    m_ctx.bytesInLine = 8 * m_settings.columnCount;

    std::string currentWord;
    bool previousIsVisible = false;
    for (size_t i = 0; i < length; ++i)
    {
        if (i % m_ctx.bytesInLine == 0)
            PrintOffset(i);

        const auto currentDumpValue = boost::format("%02x ") % static_cast<uint16_t>(buffer[i]);
        m_ctx.line.dump.append(currentDumpValue.str());

        const bool currentIsVisible = isprint(buffer[i]);
        if (currentIsVisible)
        {
            m_ctx.line.ascii += buffer[i];
            currentWord += buffer[i];
        }
        else
        {
            m_ctx.line.ascii += m_settings.placeHolder;
        }
        if (!previousIsVisible && currentIsVisible)
        {

            cout << endl;
        }

        if (i % 8 == 7)
        {
            m_ctx.line.dump.append(" ");
            if (m_settings.isSpaceBetweenBytes)
                m_ctx.line.ascii.append(" ");
        }
        if ((i % m_ctx.bytesInLine == m_ctx.bytesInLine - 1) || (i == length - 1))
        {
            if (i == length - 1)
            {
                size_t restOfDumpLine = 3 * (m_ctx.bytesInLine - length % m_ctx.bytesInLine);
                restOfDumpLine += 3 * m_settings.isSpaceBetweenBytes;
                std::string spaces(restOfDumpLine, ' ');
                m_ctx.line.dump.append(spaces);
            }
            cout << m_ctx.line.dump;
            cout << m_ctx.line.ascii << endl;

            m_ctx.line = hexdump::Line();
        }
        previousIsVisible = currentIsVisible;
    }

    PrintSeparator();
    cout << "length=" << length << "\n";
}

void Dumper::Print2(void* bufferVoid, size_t length)
{
    PrintSeparator();

    if (m_settings.cArray)
    {
        PrintArray(bufferVoid, length);
        return;
    }
    if (m_settings.only)
    {
        PrintOnlyVisible(bufferVoid, length);
        return;
    }
    if (m_settings.compress)
    {
        PrintCompressInvisible(bufferVoid, length);
        return;
    }
    if (m_settings.single)
    {
        PrintRemoveFirstInvisible(bufferVoid, length);
        return;
    }

    auto buffer = static_cast<uint8_t*>(bufferVoid);

    const size_t columnSize = 8 * m_settings.columnCount;
    const size_t mod = length % columnSize;
    const size_t rest = mod ? (columnSize - mod) : 0;

    bool isPreviousVisible = false;
    size_t currentWordLength = 0;
    for (size_t i = 0; i < length + rest; ++i)
    {
        bool isCurrentVisible = false;
        // Offset
        if (i % columnSize == 0)
        {
            PrintOffset(i);
        }
        else if (m_settings.isShowDump && i % 8 == 0)
            cout << " "; // new column

        // Dump
        if (m_settings.isShowDump)
        {
            if (i < length)
                cout << boost::format("%02x ") % static_cast<uint16_t>(buffer[i]); // hex data
            else
                cout << "   "; // end of block, aligning for ASCII hexdump
        }
        // ASCII
        if (i % columnSize == (columnSize - 1))
        {
            cout << " ";
            for (size_t j = i - (columnSize - 1); j <= i; ++j)
            {
                if (m_settings.isSpaceBetweenBytes && j % 8 == 0)
                    cout << " "; // new column

                if (j >= length)
                {
                    break; // end of block
                }
                else
                {
                    isCurrentVisible = isprint(buffer[j]);
                    if (isCurrentVisible)
                    {
                        if (currentWordLength == 0 && j != 0 && m_settings.newline)
                        {
                            cout << endl;
                            size_t spacesLength = OffsetLength() + HexDumpLength();
                            if (m_settings.isShowDump && m_settings.offset != Offset::None)
                                --spacesLength;
                            std::string spaces(spacesLength, ' ');
                            cout << spaces;
                        }

                        cout << buffer[j];
                        ++currentWordLength;
                    }
                    else
                    {
                        if (j + 1 < length && isPreviousVisible && isprint(buffer[j + 1]) && m_settings.wordwide)
                        {
                            cout << m_settings.widePlaceHolder;
                        }
                        else
                        {
                            currentWordLength = 0;
                            cout << m_settings.placeHolder;
                        }
                    }
                }
                isPreviousVisible = isCurrentVisible;
            }
            cout << endl;
        }
    }
    PrintSeparator();
    cout << "length=" << length << " (address=" << bufferVoid << ")\n";
}

void Dumper::PrintOffset(size_t i) const
{
    switch (m_settings.offset)
    {
        case (Offset::Dec):
            cout << boost::format("%5d: ") % i;
            return;
        case (Offset::Hex):
            cout << boost::format("0x%04x: ") % i;
            return;
        case (Offset::Both):
            cout << boost::format("%5d'0x%04x: ") % i % i;
            return;
        case Offset::None:
        case Offset::Unknown:
            return;
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

void Dumper::PrintSeparator() const
{
    cout << "================================" << endl;
}

} // namespace hexdump
