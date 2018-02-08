#pragma once

#include "dumper_settings.h"
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

    void SetSettings(DumperSettings settings);

    DumperSettings m_settings;

private:
    Ctx m_ctx;

    Keys m_keys;

    const std::vector<TheKey*> m_keysPtrs = {&m_keys.key, &m_keys.hkey, &m_keys.hkeyFrom, &m_keys.hkeyTill};

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
    void AppendCurrentDumpLine(const uint8_t* buffer, size_t index, utilities::Color currentColor);


    void AppendCurrentAsciiLine(const uint8_t* buffer, size_t index, const IsVisible& isVisible, utilities::Color currentColor);
    std::string GetOffsetFromIndex(size_t i) const;

    std::string GetDumpValue(uint8_t value) const;

    IsVisible IsCharVisible(const uint8_t* buffer, const size_t index) const;
    bool IsNextWordStart(size_t index, const IsVisible& isVisible) const;
    size_t PositionInLine(size_t index) const;

    bool IsPositionLastInColumn(size_t index) const;
    bool IsEndOfCurrentLine(size_t index) const;
    uint8_t* ShiftBeginOfBufferAndResults(uint8_t* buffer, DumperSettings& settings);

    utilities::Color GetColor(size_t index, const uint8_t currentValue) const;

    bool IsLineSkipped(Range range);
    bool IsLineNearKeys(const Range& range, size_t position, size_t keySize) const;

    uint8_t* FindKeysAndShiftStartOfBuffer(const size_t bufferLength, uint8_t* buffer);

};

}; // namespace dump
