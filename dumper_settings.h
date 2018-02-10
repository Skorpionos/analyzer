#pragma once

#include "types.h"

namespace dump::defaultvalues
{
constexpr size_t ColumnCountDefault  = 6;
constexpr size_t BytesCountInGroup   = 8;
constexpr size_t BytesCountBeforeKey = 0;
constexpr size_t BytesCountAfterKey  = 96;
} // dump::defaultvalues

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

    unsigned long shift = range.begin;

    unsigned long countBytesBeforeKey = defaultvalues::BytesCountBeforeKey;
    unsigned long countBytesAfterKey = defaultvalues::BytesCountAfterKey;
    unsigned long columnCount = defaultvalues::ColumnCountDefault;
    unsigned long bytesInGroup = defaultvalues::BytesCountInGroup;
    unsigned long bytesInLine = columnCount * bytesInGroup;

    bool isCArray = false;
    bool newLine = false;
    bool useWideChar = false;
    bool ladder = false;
    bool useRelativeAddress = false;
    bool skipTextWithoutKeys = false;
    bool isShowEmptyLines = true;

    std::string hkeyFrom;
    std::string hkeyTill;

    StringVector hkeyValues;
    StringVector keyValues;

    unsigned long countBytesAfterHkeyFrom = 0;
    bool printZeroAsGrey = true;
};

} // namespace dump

