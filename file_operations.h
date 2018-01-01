#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>

bool ReadBuffer(std::string fileName, char** buffer, size_t& size);