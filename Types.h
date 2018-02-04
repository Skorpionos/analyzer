#pragma once

#include "utilities.h"

#include <boost/format.hpp>
#include <boost/algorithm/string/split.hpp>
#include <map>
#include <algorithm>
#include <sstream>

using StringVector = std::vector<std::string>;
using Uint8Vector = std::vector<uint8_t>;
using SizeVector = std::vector<size_t>;

struct Range
{
    size_t begin = 0;
    size_t end = 0;
    size_t GetSize() {return end - begin + 1;}
};

