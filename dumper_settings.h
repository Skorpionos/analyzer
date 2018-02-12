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
    struct IsShow
    {
        bool offset = true;
        bool dump = true;
        bool ascii = true;
        bool debug = false;
    };

    IsShow isShow;

    bool isShowDetail = false;

    bool isSpaceBetweenAsciiBytes = false;
    bool isSpaceBetweenDumpBytes = true;

    OffsetTypes offset = OffsetTypes::Dec;

    char placeHolder = '-';
    char zeroPlaceHolder = '.';
    char widePlaceHolder = ' ';

    // TODO use range in Ctx
    Range range = {0, 0};

    size_t shift = range.begin;

    size_t countBytesBeforeKey  = defaultvalues::BytesCountBeforeKey;
    size_t countBytesAfterKey   = defaultvalues::BytesCountAfterKey;
    size_t columnCount          = defaultvalues::ColumnCountDefault;
    size_t bytesInGroup         = defaultvalues::BytesCountInGroup;
    size_t bytesInLine = columnCount * bytesInGroup;

    bool isArray = false;
    bool useNewLine = false;
    bool useWideChar = false;
    bool ladder = false;
    bool useRelativeAddress = false;
    bool skipTextWithoutKeys = false;
    bool isShowEmptyLines = true;

    std::string hkeyFrom;
    std::string hkeyTill;

    std::string hkeyBreak;

    StringVector hkeyValues;
    StringVector keyValues;

    size_t countBytesAfterHkeyFrom = 0;
    bool printZeroAsGrey = true;
};

} // namespace dump

