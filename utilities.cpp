#include "utilities.h"

namespace utilities
{

void PrintSeparator()
{
    std::cout << "================================" << std::endl;
}

void PrintEmptyLine(bool isPrint, size_t skippedLinesCount)
{
    if (!isPrint)
        return;

    std::cout << "...";
    if (skippedLinesCount > 1)
        std::cout <<  " <" << skippedLinesCount << ">";
    std::cout << "\n";
}

void PrintFoundKeyResults(const std::string& keyName, const SizeVector& positionResults, size_t startOffset)
{
    if (keyName.empty())
        return;

    std::cout << "for key '" << keyName << "' found " << positionResults.size() << " results: ";
    for (const size_t& position : positionResults)
    {
        int64_t relativePosition = position - startOffset;
        std::cout << relativePosition;
        if (startOffset != 0)
            std::cout << "/" << position;
        std::cout << " ";
    }
    std::cout << "\n";
}

} // utilities