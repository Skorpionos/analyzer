#pragma once

#include "Types.h"

#include <iostream>
#include <string>

namespace utilities
{

enum Color
{
    Normal, Green, Red, Grey, Yellow, Blue
};

const StringVector ColorCode =
        {
                "\033[0m",
                "\033[1;37;42m",
                "\033[1;37;41m",
                "\033[1;30;40m",
                "\033[1;30;103m",
                "\033[1;37;44m",

        };


void PrintSeparator();

void PrintEmptyLine(bool isPrint, size_t skippedLinesCount);

void
PrintFoundKeyResults(const std::string& keyName, const SizeVector& positionResults, const size_t startOffset,
                     const bool useOffsetDetailedInfo, const Color color);

void PrintRange(const Range range, const size_t shift, const bool detailed);

} // utilities
