#pragma once

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>

#include <algorithm>
#include <map>
#include <utility>
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
