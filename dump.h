#pragma once

#include "utilities.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <iterator>
#include <map>

#include <iostream>
#include <sstream>
#include <string>
#include <cstddef>

namespace dump::defaultvalues
{
constexpr size_t ColumnCountDefault = 6;
constexpr size_t ByteCountInGroup = 8;
constexpr size_t countBytesBeforeKey = 0;
constexpr size_t countBytesAfterKey = 96;
} // dump::defaultvalues

namespace dump::size
{
constexpr size_t CharsForOneByte = 3;
constexpr size_t AdditionalCharsForCArray = 3;
} // namespace dump::size

namespace dump
{
struct Range
{
    size_t begin = 0;
    size_t end = 0;
};

enum class OffsetTypes
{
    Hex,
    Dec,
    Both,
    None,
    Unknown
};

OffsetTypes GetOffsetType(const std::string& offset);

enum Color
{
    Normal,
    Green,
    Red
};

const std::vector<std::string> colorString =
{
    "\033[0m",
    "\033[1;37;42m",
    "\033[1;37;41m"
};

struct DumperSettings
{
    bool isShowOffset = true;
    bool isShowDump = true;
    bool isShowAscii = true;
    bool isShowDebug = false;
    bool isSpaceBetweenAsciiBytes = false;
    bool isSpaceBetweenDumpBytes = true;

    OffsetTypes offset = OffsetTypes::Dec;
    bool detailedOffset = false;

    char placeHolder = '-';
    char zeroPlaceHolder = '.';
    char widePlaceHolder = ' ';

    size_t begin = 0;
    size_t size = 0;
    size_t end = 0;

    size_t startOffset = begin;

    size_t countBytesBeforeKey = dump::defaultvalues::countBytesBeforeKey;
    size_t countBytesAfterKey = dump::defaultvalues::countBytesAfterKey;

    size_t columnCount = dump::defaultvalues::ColumnCountDefault;
    size_t bytesInGroup = dump::defaultvalues::ByteCountInGroup;
    size_t bytesInLine = columnCount * bytesInGroup;

    bool isCArray = false;
    bool newLine = false;
    bool useWideChar = false;
    bool ladder = false;
    bool useRelativeAddress = false;
    bool skipTextWithoutKeys = false;
    bool isShowEmptyLines = true;

    std::string key;
    std::string hkey;
    std::string from;
    std::string till;
};

struct Line
{
    std::string offset;
    std::string dump;
    std::string ascii;
    std::string indent;
    std::string debug;
};

struct IsVisible
{
    bool preprevious = false;
    bool previous = false;
    bool current = false;
    bool next = false;
};

class Ctx
{
public:
    Line line {};
    Line savedLine {};
    Range range = {0, 0};
    bool previousIsSkipped = false;
    size_t skipLinesCount = 0;

    void Print(bool isOffsetVisible, bool isDumpVisible, bool isAsciiVisible, bool isCArray, bool isDebugVisible);
};

class Dumper
{
public:
    explicit Dumper(DumperSettings settings);

    void Generate(void* bufferVoid, size_t bufferLength);

    void SetSettings(dump::DumperSettings settings);

private:
    DumperSettings m_settings;
    Ctx m_ctx;

    std::vector<size_t> m_keyPositionResults;
    std::vector<size_t> m_hkeyPositionResults;
    size_t m_countBytesInHexKey;

private:

    void ClearLine();
    void GenerateImpl(uint8_t* buffer);

    void PrepareFirstLine(size_t index);
    void PrintLineAndIntend(size_t index);
    void PrintAndClearLine(const bool isNewGroupAfterSkippedLines);
    void CompleteCurrentDumpLine();

    void PrintFoundKeyResults(const std::string& keyName, const std::vector<size_t>& positionResults) const;

    bool FindIndexForKey(const uint8_t* buffer, size_t bufferLength, std::string key,
            std::vector<size_t>& resultPositions);

    std::string GetSpacesForBeginOfDumpLine (const size_t positionInLine) const;
    std::string GetSpacesForBeginOfAsciiLine(const size_t positionInLine);
    std::string GetSpacesForRestOfDumpLine  (const size_t positionInLine) const;

    void AppendCurrentDumpLine(const uint8_t* buffer, const size_t index, Color currentColor);
    void AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, Color currentColor);

    std::string GetOffsetFromIndex(size_t i) const;

    std::string GetDumpValue(uint8_t value) const;
    IsVisible IsCharVisible(const uint8_t* buffer, const size_t index) const;
    bool IsNextWordStart(size_t index, const IsVisible& isVisible) const;

    size_t PositionInLine(size_t index) const;
    bool IsPositionLastInColumn(size_t index) const;
    bool IsEndOfCurrentLine(size_t index) const;

    uint8_t* ShiftStartOffset(uint8_t* buffer);

    bool FindRangeForPairOfKeys(uint8_t* buffer, size_t bufferLength, std::string hexKeyFrom, std::string hexKeyTill,
            Range& range);

    bool FindIndexForBytesKey(const uint8_t* buffer, size_t bufferLength, const uint8_t* key, size_t keyLength,
            std::vector<size_t>& resultPositions);

    void GetHexKeyBytes(const std::vector<std::string>& words, uint8_t* hexKeyBytes) const;

    void FindIndexForHexKey(const uint8_t* buffer, size_t bufferLength, std::string& hexKey,
            std::vector<size_t>& foundPositions, size_t& countBytesInKey);

    Color GetColor(size_t index) const;
    bool IsLineSkipped(Range range);
    bool IsLineNearKeys(const Range& range, size_t position, size_t keySize) const;
};

}; // namespace dump
