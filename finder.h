#pragma once

#include "types.h"

#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <map>
#include <sstream>

namespace finder
{

struct Key;

class Finder
{
public:
    virtual SizeVector Find(const uint8_t* buffer, Range range, Key& key) = 0;

    Range FindRange(uint8_t* buffer, Range range, const std::string& hexFromValue, const std::string& hexTillValue);


protected:
    virtual size_t FindFirst(const uint8_t* buffer, Range range, std::string hexKeyValue) = 0;
    virtual size_t FindLast(const uint8_t* buffer, Range range, std::string hexKeyValue) = 0;

    size_t FindFirstForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes);
    size_t FindLastForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes);

    SizeVector FindIndexesForBytesKey(const uint8_t* buffer, Range range, const Uint8Vector& keyBytes);

};

class KeyFinder : public Finder
{
public:
    SizeVector Find(const uint8_t* buffer, Range range, Key& key) override;
    size_t FindFirst(const uint8_t* buffer, Range range, std::string hexKeyValue) override {};
    size_t FindLast(const uint8_t* buffer, Range range, std::string hexKeyValue) override {};
};

class HexKeyFinder : public Finder
{
public:
    SizeVector Find(const uint8_t* buffer, Range range, Key& hexKey) override;
    size_t FindFirst(const uint8_t* buffer, Range range, std::string hexKeyValue) override;
    size_t FindLast(const uint8_t* buffer, Range range, std::string hexKeyValue) override;
};


struct Key
{
    SizeVector Find(const uint8_t* buffer, Range range, Key& hexKey)
    {
        return finder->Find(buffer, range, hexKey);
    }

    Range FindRange(uint8_t* buffer, Range range, const std::string& fromValue, const std::string& tillValue)
    {
        return finder->FindRange(buffer, range, fromValue, tillValue);
    }

    size_t id = 0;
    std::string value;
    SizeVector results;
    size_t length = 0;
    std::shared_ptr<Finder> finder = nullptr;

    explicit Key(const size_t id_ = 0, std::string value_ = "", std::shared_ptr<Finder> finder_ = std::make_shared<HexKeyFinder>()) :
        id(id_),
        value(std::move(value_)),
        finder(std::move(finder_))
        {}
};

using SharedKey = std::shared_ptr<Key>;

using SharedKeysVector = std::vector<std::shared_ptr<Key>>;

Uint8Vector GetHexBytesFromStringVector(const StringVector& words);

} // namespace finder
