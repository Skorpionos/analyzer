#pragma once

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include "file_operations.h"
#include "dump.h"

bool ReadOptions(int argc, char** argv, std::string& fileName, hexdump::DumperSettings& settings);

//constexpr const char* defaultFileName = "/home/Skopintsev/Traffic/Emerson/6/6-04-01-01-int8_-128.pcap";
