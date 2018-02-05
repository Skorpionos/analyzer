#pragma once

#include "dump.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

bool ReadOptions(int argc, char** argv, std::string& fileName, dump::DumperSettings& settings);
