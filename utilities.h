#pragma once

#include "types.h"

namespace utilities
{

enum Color
{
    Blue,
    Green,
    Red,
    Yellow,
    Grey,
    Normal
};

constexpr size_t differentColorsCount = 4;

const StringVector ColorAnsiCode =
{
    "\033[1;37;44m",
    "\033[1;37;42m",
    "\033[1;37;41m",
    "\033[1;30;103m",
    "\033[1;30;40m",
    "\033[0m"
};

Color GetColorIndex(const size_t colorIndex);

void PrintSeparator();

void PrintEmptyLine(bool isPrint, size_t skippedLinesCount);

void PrintFoundKeyResults(const SharedKey key, const bool useOffsetDetailedInfo, const size_t startOffset);

void PrintRange(const Range range, const size_t shift, const bool detailed);

} // utilities
