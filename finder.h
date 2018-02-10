#pragma once

#include "types.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <map>
#include <sstream>

namespace finder
{

struct Key
{
    size_t id = 0;
    std::string value;
    SizeVector results;
    size_t length = 0;

    explicit Key(const size_t id_ = 0, std::string value_ = "") : id(id_), value(std::move(value_)) {}
};

struct SomeKeys
{
    Key hkeyFrom;
    Key hkeyTill;
};

using SharedKey = std::shared_ptr<Key>;
using SharedKeysVector = std::vector<std::shared_ptr<Key>>;

Uint8Vector GetHexBytesFromStringVector(const StringVector& words);

SizeVector FindIndexesForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes);

SizeVector FindIndexesForKey(const uint8_t* buffer, Range range, Key& key);

SizeVector FindIndexesForHexKey(const uint8_t* buffer, Range range, Key& hexKey);

Range FindRangeForPairOfKeys(uint8_t* buffer, Range range, Key& hexKeyFrom, Key& hexKeyTill);

}; // namespace finder
