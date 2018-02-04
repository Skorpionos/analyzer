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

SizeVector FindIndexesForBytesKey(const uint8_t* buffer,
                                  const size_t bufferLength,
                                  const Uint8Vector& keyBytes);

SizeVector FindIndexesForKey(const uint8_t* buffer,
                             const size_t bufferLength,
                             const std::string& key);

SizeVector FindIndexesForHexKey(const uint8_t* buffer,
                                const size_t bufferLength,
                                const std::string& hexKey,
                                size_t& bytesCountInHexKey);

Range FindRangeForPairOfKeys(uint8_t* buffer, size_t bufferLength, std::string hexKeyFrom, std::string hexKeyTill);

}; // namespace finder
