#pragma once

#include "types.h"

namespace utilities
{

enum Color
{
    Normal,
    Red,
    Green,
    Blue,
    Yellow,
    Grey
};

const StringVector ColorAnsiCode = // TODO add function GetAnsiCode
{
    "\033[0m",
    "\033[1;37;41m",
    "\033[1;37;42m",
    "\033[1;37;44m",
    "\033[1;30;103m",
    "\033[1;30;40m",
};

void PrintSeparator();

void PrintEmptyLine(bool isPrint, size_t skippedLinesCount);

void PrintFoundKeyResults(const TheKey& key, const Color color, const bool useOffsetDetailedInfo,
                          const size_t startOffset);

void PrintRange(const Range range, const size_t shift, const bool detailed);

void PrintKeysResults(const Keys& keys, const bool showDetailed, const size_t shift);

} // utilities
