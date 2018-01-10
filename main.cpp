#include "dump.h"
#include "file_operations.h"
#include "read_options.h"
#include "utilities.h"

using namespace std;

int main(int argc, char* argv[])
{
    hexdump::DumperSettings formatSettings;

    std::string fileName;

    if (!ReadOptions(argc, argv, fileName, formatSettings))
        return 0;

    char* buffer = nullptr;
    size_t size = 0;

    ReadBuffer(fileName, &buffer, size);

    hexdump::Dumper dumper(formatSettings);


    PrintSeparator();
    dumper.Print(buffer, size);
    PrintSeparator();
    cout << "length=" << size << "\n";

    delete[] buffer;
    return 0;
}


























