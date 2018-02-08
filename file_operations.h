#pragma once

#include <fstream>
#include <iostream>

namespace files_operations
{

bool ReadBuffer(std::string fileName, char** buffer, size_t& size);

} // namespace files_operations
