#pragma once

#include "Types.h"

#include <iostream>
#include <string>

namespace utilities
{

void PrintSeparator();
void PrintEmptyLine(bool isPrint, size_t skippedLinesCount);

void PrintFoundKeyResults(const std::string& keyName, const SizeVector& positionResults, size_t startOffset);

} // utilities
