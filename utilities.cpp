#include "utilities.h"

namespace utilities
{

void PrintSeparator()
{
    std::cout << "================================" << std::endl;
}

void PrintRange(const Range range, const size_t shift, const bool detailed)
{
    std::cout << "address <";
    std::cout << range.begin;
    if (detailed && shift != 0)
        std::cout << "/"<< range.begin + shift;
    std::cout << "...";
    std::cout << range.end;
    if (detailed && shift != 0)
        std::cout << "/"<< range.end + shift;
    std::cout << ">";
    std::cout << ", size:" << range.GetSize() << "\n";
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

void PrintFoundKeyResults(const std::string& keyName, const SizeVector& indexResults, const size_t startOffset,
                          const bool useOffsetDetailedInfo, const Color color)
{
    if (keyName.empty())
        return;

    std::cout << "for key '" << CololAnsiCode[color] << keyName << CololAnsiCode[Color::Normal] << "' found " << indexResults.size() << " results: ";
    for (const size_t& position : indexResults)
    {
        std::cout << position;
        if (startOffset != 0 && useOffsetDetailedInfo)
            std::cout << "/" << position + startOffset;
        std::cout << " ";
    }
    std::cout << "\n";
}

void PrintKeysResults(const std::string& keyFrom, const std::string& keyTill, const std::string& key,
                      const std::string& hkey, const size_t shift, const bool showDetailed,
                      const SizeVector& fromIndexResults, const SizeVector& tillIndexResults,
                      const SizeVector& keyIndexResults, const SizeVector& hkeyIndexResults)
{
    PrintFoundKeyResults(keyFrom, fromIndexResults, shift, showDetailed, Color::Blue);
    PrintFoundKeyResults(keyTill, tillIndexResults, shift, showDetailed, Color::Yellow);
    PrintFoundKeyResults(key, keyIndexResults, shift, showDetailed, Color::Red);
    PrintFoundKeyResults(hkey, hkeyIndexResults, shift, showDetailed, Color::Green);
}

} // utilities