#pragma once

#include "Types.h"
#include "dump.h"

#include <iostream>
#include <string>

namespace utilities
{

void PrintSeparator();

void PrintEmptyLine(bool isPrint, size_t skippedLinesCount);

void
PrintFoundKeyResults(const std::string& keyName, const SizeVector& positionResults, const size_t startOffset,
                     const bool useOffsetDetailedInfo);

void PrintRange(const Range range, const size_t shift, const bool detailed);

} // utilities
