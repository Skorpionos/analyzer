#pragma once

#include "dumper_settings.h"

#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

namespace command_line
{

bool ReadOptions(int argc, char** argv, std::string& fileName, dump::DumperSettings& settings);

} // command_line
