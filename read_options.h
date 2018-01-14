#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include "file_operations.h"
#include "dump.h"

bool ReadOptions(int argc, char** argv, std::string& fileName, dump::DumperSettings& settings);
