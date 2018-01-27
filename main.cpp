#include "dump.h"
#include "file_operations.h"
#include "read_options.h"
#include "utilities.h"

using namespace std;

int main(int argc, char* argv[])
{
    dump::DumperSettings settings;

    std::string fileName;

    if (!ReadOptions(argc, argv, fileName, settings))
        return 0;

    char* buffer = nullptr;

    size_t size = 0;

    ReadBuffer(fileName, &buffer, size);

    dump::Dumper dumper(settings);

    dumper.Generate(buffer, size);

    delete[] buffer;

    return 0;
}


























