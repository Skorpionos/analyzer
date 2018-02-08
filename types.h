#pragma once

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <map>
#include <algorithm>
#include <sstream>
#include <iostream>

using StringVector = std::vector<std::string>;
using Uint8Vector = std::vector<uint8_t>;
using SizeVector = std::vector<size_t>;

struct Range
{
    size_t begin = 0;
    size_t end = 0;
    size_t GetSize() const {return end - begin + 1;}
};

struct TheKey
{
    size_t index = 0;
    std::string value;
    SizeVector results;
    size_t length = 0;

    TheKey(const size_t id_, const std::string value_ = "") : index(id_), value(value_) {}
};

struct Keys
{
    TheKey hkeyFrom {0};
    TheKey hkeyTill {1};
    TheKey key      {2};
    TheKey hkey     {3};
    std::vector<TheKey> hexKeysVector;
};

