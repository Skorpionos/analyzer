#pragma once

#include "dumper_settings.h"
#include "finder.h"
#include "utilities.h"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <map>
#include <sstream>

namespace dump::size
{
constexpr size_t CharsForOneByte = 3;
constexpr size_t AdditionalCharsForCArray = 3;
}

namespace dump
{

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

class Dumper
{
public:
    explicit Dumper(DumperSettings settings);

    void Generate(void* bufferVoid, size_t bufferLength);

    DumperSettings m_settings;

private:
    struct Ctx
    {
        Line line {};
        Range range = {0, 0};
        bool previousLineWasSkipped = false;
        size_t skipLinesCount = 0;
        size_t deltaIndex = 0;

        void Print(const DumperSettings::IsShow& isShow, bool isArray);
    };

    Ctx m_ctx;

//    finder::RangeKeys m_rangeHKeys;

    finder::SharedKeysVector m_keys = {};
    size_t m_hkeyBreakIndex = 0;

private:

    void ClearLine();
    void GenerateImpl(uint8_t* buffer);

    void PrepareFirstLine(size_t index);
    void PrintLineAndIntend(size_t index);
    void PrintAndClearLine(bool isNewGroupAfterSkippedLines);
    void CompleteCurrentDumpLine();

    std::string GetSpacesForBeginOfDumpLine (size_t positionInLine) const;

    std::string GetSpacesForBeginOfAsciiLine(size_t positionInLine);
    std::string GetSpacesForRestOfDumpLine  (size_t positionInLine) const;
    void AppendCurrentDumpLine(uint8_t dumpByte, size_t index, utilities::Color currentColor);
    void AppendCurrentAsciiLine(uint8_t dumpByte, size_t index, const IsVisible& isVisible,
                                    utilities::Color currentColor);
    std::string GetOffset(const size_t index) const;

    std::string GetDumpValueSymbol(uint8_t value) const;

    IsVisible IsCharVisible(const uint8_t* buffer, size_t index) const;
    bool IsNextWordStart(size_t index, const IsVisible& isVisible) const;
    size_t PositionInLine(size_t index) const;

    bool IsPositionLastInColumn(size_t index) const;
    bool IsEndOfCurrentLine(size_t index) const;
    uint8_t* ShiftBeginOfBufferAndResults(uint8_t* buffer, DumperSettings& settings);

    utilities::Color GetColor(size_t index, uint8_t currentValue) const;

    bool IsLineSkipped(Range range);
    bool IsLineNearKeys(const Range& range, size_t position, size_t keySize) const;

    uint8_t* FindKeysAndShiftStartOfBuffer(size_t bufferLength, uint8_t* buffer);

    void AddKeyInVector(const std::string& keyValue, finder::SharedKeysVector& keysVector) const;

    bool CheckBreakPosition(const size_t index);

    void PrintLineIfEndOfLine(size_t index);
};

}; // namespace dump
