#pragma once

#include "dump.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <map>
#include <sstream>

namespace finder
{

Uint8Vector GetHexBytesFromStringVector(const StringVector& words);

SizeVector FindIndexesForBytesKey(const uint8_t* buffer, const Range range, const Uint8Vector& keyBytes);

SizeVector FindIndexesForKey(const uint8_t* buffer, const Range range, const std::string& key);

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const Range range, const std::string& hexKey, size_t& bytesCountInHexKey);

Range FindRangeForPairOfKeys(uint8_t* buffer, const Range range,
                             const std::string& hexKeyFrom, const std::string& hexKeyTill,
                             dump::OneKeyFoundResult& fromKeyFoundResult, dump::OneKeyFoundResult& tillKeyFoundResult);

}; // namespace finder
