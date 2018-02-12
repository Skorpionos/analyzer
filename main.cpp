#include "dump.h"
#include "file_operations.h"
#include "read_options.h"

int main(int argc, const char* const* argv)
{
    dump::DumperSettings settings;

    std::string fileName;

    if (!command_line::ReadOptions(argc, argv, fileName, settings))
        return 0;

    char* buffer = nullptr;

    size_t size = 0;

    files_operations::ReadBuffer(fileName, &buffer, size);

    dump::Dumper dumper(settings);

    dumper.Generate(buffer, size);

    delete[] buffer;

    return 0;
}
