#pragma once

#include <cstddef>
#include <iostream>
#include <string>
#include <algorithm>

namespace dump
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

    size_t columnCount = dump::ColumnCountDefault;

    bool cArray = false;
    bool only = false;
    bool single = false;
    bool compress = false;
    bool newline = false;
    bool wordwide = false;
    bool ladder = true;

    size_t length = 0;

    size_t bytesInLine = 0;
};

struct Line
{
    std::string offset;
    std::string dump;
    std::string ascii;
    std::string debug;
};

struct Ctx
{
    Line line {};

    void Clear()
    {
        line = Line();
    }
    void Print()
    {
        std::cout << line.offset << line.dump << line.ascii << "\t" << line.debug << std::endl;
    }
};

class Dumper
{
public:
    explicit Dumper(DumperSettings settings);

    void Print(void* bufferVoid, size_t length);

public:
    void SetSettings(dump::DumperSettings settings);

private:
    std::string GetOffsetFromIndex(size_t i) const;
    size_t OffsetLength();
    size_t HexDumpLength() const;

private:
    DumperSettings m_settings;
    Ctx m_ctx;

    std::string GetSpacesForBeginOfLine(const size_t positionInLine) const;
    std::string GetSpacesForRestOfLine(const size_t positionInLine) const;
    bool isPositionLastInColumn(size_t index) const;
    size_t PositionInLine(size_t index) const;
};

}; // namespace dump
