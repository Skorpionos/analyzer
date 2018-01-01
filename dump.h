#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <algorithm>

namespace hexdump
{
constexpr size_t ColumnCountDefault = 4;

enum class Offset
{
    Hex,
    Dec,
    Both,
    None,
    Unknown
};

Offset GetOffsetType(const std::string& offset);

struct DumperSettings
{
    bool isSpaceBetweenBytes = true;
    bool isShowDump = true;
    Offset offset = Offset::Hex;
    char placeHolder = '-';
    char widePlaceHolder = ' ';
    size_t columnCount = ColumnCountDefault;
    bool cArray = false;
    bool only = false;
    bool single = false;
    bool compress = false;
    bool newline = false;
    size_t length = 0;
    bool wordwide = false;
};

struct Line
{
    std::string offset;
    std::string dump;
    std::string ascii;
};

struct Ctx
{
    Line line {};
    size_t bytesInLine = 0;
};

class Dumper
{
public:
    Dumper(DumperSettings settings);

    void Print(void* bufferVoid, size_t length);
    void Print2(void* bufferVoid, size_t length);
    void PrintArray(void* bufferVoid, size_t length);
    void PrintOnlyVisible(void* bufferVoid, size_t length);
    void PrintCompressInvisible(void* bufferVoid, size_t length);
    void PrintRemoveFirstInvisible(void* bufferVoid, size_t length);

public:
    void SetSettings(hexdump::DumperSettings settings);

private:
    void PrintOffset(size_t i) const;
    size_t OffsetLength();
    size_t HexDumpLength() const;

private:
    DumperSettings m_settings;
    Ctx m_ctx;
    void PrintSeparator() const;
};

}; // namespace hexdump
