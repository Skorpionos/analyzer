#pragma once

#include "Types.h"
#include "utilities.h"

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

struct DumperSettings
{
    bool isShowOffset = true;
    bool isShowDump = true;
    bool isShowAscii = true;
    bool isShowDebug = false;
    bool isSpaceBetweenAsciiBytes = false;
    bool isSpaceBetweenDumpBytes = true;

    OffsetTypes offset = OffsetTypes::Dec;
    bool showDetailed = false;

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
    std::string keyFrom;
    std::string keyTill;

    size_t countBytesAfterHkeyFrom = 0;
    bool printZeroAsGrey = true;
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

struct OneKeyFoundResult
{
    SizeVector vector;
    size_t length;
};


struct KeysResults
{
    OneKeyFoundResult key;
    OneKeyFoundResult hkey;
    OneKeyFoundResult from;
    OneKeyFoundResult till;
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

    KeysResults m_foundKeys;

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
    uint8_t* ShiftBeginOfBufferAndResults(uint8_t* buffer, DumperSettings& settings);

    Color GetColor(size_t index, const uint8_t currentValue) const;

    bool IsLineSkipped(Range range);
    bool IsLineNearKeys(const Range& range, size_t position, size_t keySize) const;

    uint8_t* FindKeysAndShiftStartOfBuffer(const size_t bufferLength, uint8_t* buffer);

};

}; // namespace dump
