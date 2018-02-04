#include "file_operations.h"

bool ReadBuffer(std::string fileName, char** buffer, size_t& size)
{
    std::cout << "analyze file: " << fileName << std::endl;

    std::ifstream file(fileName, std::ios_base::binary | std::ios_base::in | std::ios_base::ate);

    if (!file.is_open())
    {
        std::cout << "File is not open" << std::endl;
        return false;
    }
    size = static_cast<size_t>(file.tellg());

    try
    {
        *buffer = new char[size];
    }
    catch (...)
    {
        std::cout << "Not enough memory" << std::endl;
        return false;
    }

    file.seekg(0, std::ios_base::beg);
    file.read(*buffer, size);
    file.close();

    return true;
}