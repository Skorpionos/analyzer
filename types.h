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

    TheKey(const size_t index) : index(index) {}
};

//enum KeyType
//{
//    key = 0,
//    hkey = 1,
//    hkeyFrom = 2,
//    hkeyTill = 3
//
//};

struct Keys
{
    TheKey key      {1};
    TheKey hkey     {2};
    TheKey hkeyFrom {3};
    TheKey hkeyTill {4};
};

