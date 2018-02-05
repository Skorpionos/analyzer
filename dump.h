#pragma once

#include "utilities.h"
#include "Types.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <map>
#include <sstream>
#include <string>

namespace dump::defaultvalues
{
constexpr size_t ColumnCountDefault = 6;
constexpr size_t BytesCountInGroup = 8;
constexpr size_t BytesCountBeforeKey = 0;
constexpr size_t BytesCountAfterKey = 96;
}

namespace dump::size
{
constexpr size_t CharsForOneByte = 3;
constexpr size_t AdditionalCharsForCArray = 3;
}

namespace dump
{

enum class OffsetTypes
{
    Hex, Dec, Both, None, Unknown
};

OffsetTypes GetOffsetType(const std::string& offset);

enum Color
{
    Normal, Green, Red
};

const StringVector colorString =
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

    Range range = {0, 0};

    size_t shift = range.begin;

    size_t countBytesBeforeKey = defaultvalues::BytesCountBeforeKey;
    size_t countBytesAfterKey  = defaultvalues::BytesCountAfterKey;
    size_t columnCount         = defaultvalues::ColumnCountDefault;
    size_t bytesInGroup        = defaultvalues::BytesCountInGroup;
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
    bool previous    = false;
    bool current     = false;
    bool next        = false;
};

class Ctx
{
public:
    Line line {};
    Range range = {0, 0};
    bool previousLineWasSkipped = false;
    size_t skipLinesCount = 0;

    void Print(bool isOffsetVisible, bool isDumpVisible, bool isAsciiVisible, bool isCArray, bool isDebugVisible);
};

class Dumper
{
public:
    explicit Dumper(DumperSettings settings);

    void Generate(void* bufferVoid, const size_t bufferLength);

    void SetSettings(dump::DumperSettings settings);

    DumperSettings m_settings;
private:
    Ctx m_ctx;

    SizeVector m_keyPositionResults;
    SizeVector m_hkeyPositionResults;
    size_t m_countBytesInHexKey;

private:

    void ClearLine();
    void GenerateImpl(uint8_t* buffer);

    void PrepareFirstLine(size_t index);
    void PrintLineAndIntend(size_t index);
    void PrintAndClearLine(const bool isNewGroupAfterSkippedLines);
    void CompleteCurrentDumpLine();

    std::string GetSpacesForBeginOfDumpLine (const size_t positionInLine) const;

    std::string GetSpacesForBeginOfAsciiLine(const size_t positionInLine);
    std::string GetSpacesForRestOfDumpLine  (const size_t positionInLine) const;
    void AppendCurrentDumpLine(const uint8_t* buffer, size_t index, Color currentColor);

    void AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, Color currentColor);
    std::string GetOffsetFromIndex(size_t i) const;

    std::string GetDumpValue(uint8_t value) const;

    IsVisible IsCharVisible(const uint8_t* buffer, const size_t index) const;
    bool IsNextWordStart(size_t index, const IsVisible& isVisible) const;
    size_t PositionInLine(size_t index) const;

    bool IsPositionLastInColumn(size_t index) const;
    bool IsEndOfCurrentLine(size_t index) const;
    uint8_t* ShiftStartOffset(uint8_t* buffer);


    Color GetColor(size_t index) const;

    bool IsLineSkipped(Range range);
    bool IsLineNearKeys(const Range& range, size_t position, size_t keySize) const;

};

}; // namespace dump
