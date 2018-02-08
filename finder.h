#pragma once

#include "types.h"

#include <algorithm>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <map>
#include <sstream>

namespace finder
{

Uint8Vector GetHexBytesFromStringVector(const StringVector& words);

SizeVector FindIndexesForBytesKey(const uint8_t* buffer, const Range range, const Uint8Vector& keyBytes);

SizeVector FindIndexesForKey(const uint8_t* buffer, const Range range, TheKey& key);

SizeVector FindIndexesForHexKey(const uint8_t* buffer, const Range range, TheKey& hexKey);

Range FindRangeForPairOfKeys(uint8_t* buffer, const Range range, TheKey& hexKeyFrom, TheKey& hexKeyTill);

}; // namespace finder
