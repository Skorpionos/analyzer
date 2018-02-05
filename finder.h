#pragma once

#include "utilities.h"
#include "dump.h"
#include "Types.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <map>
#include <sstream>

namespace finder
{

Uint8Vector GetHexBytesFromStringVector(const StringVector& words);

SizeVector
FindIndexesForBytesKey(const uint8_t* buffer, const size_t startIndex, const size_t length, const Uint8Vector& keyBytes);

SizeVector
FindIndexesForKey(const uint8_t* buffer, const size_t start, const size_t length, const std::string& key);

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const size_t start, const size_t legth,
                                const std::string& hexKey, size_t& bytesCountInHexKey);

Range FindRangeForPairOfKeys(uint8_t* buffer, const size_t startIndex, const size_t length, const std::string& hexKeyFrom,
                             const std::string& hexKeyTill, const size_t sizeAfterFrom, SizeVector& resultsFrom,
                             SizeVector& resultsTill, size_t& countBytesInKeyFrom, size_t& countBytesInKeyTill);

}; // namespace finder
