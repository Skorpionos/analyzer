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

void PrintFoundKeyResults(const TheKey& key, const Color color, const bool useDetailedInfo, const size_t startOffset)
{
    if (key.value.empty())
        return;

    std::cout << "for key '" << ColorAnsiCode[color] << key.value << ColorAnsiCode[Color::Normal];
    std::cout << "' found " << key.results.size() << " results: ";

    for (const size_t& index : key.results)
    {
        std::cout << index;
        if (startOffset != 0 && useDetailedInfo)
            std::cout << "/" << index + startOffset;
        std::cout << " ";
    }
    std::cout << "\n";
}

void PrintKeysResults(const Keys& keys, const bool showDetailed, const size_t shift)
{
    PrintFoundKeyResults(keys.hkeyFrom, Color::Blue,   showDetailed, shift);
    PrintFoundKeyResults(keys.hkeyTill, Color::Yellow, showDetailed, shift);
    PrintFoundKeyResults(keys.key,  Color::Red,    showDetailed, shift);
    PrintFoundKeyResults(keys.hkey, Color::Green,  showDetailed, shift);
}

} // utilities