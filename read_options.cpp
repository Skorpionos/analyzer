#include "read_options.h"

bool ReadOptions(int argc, char** argv, std::string& fileName, hexdump::DumperSettings& settings)
{
    namespace po = boost::program_options;
    boost::program_options::options_description desc("Options");
    std::string addressType;
    desc.add_options()
            ("help", "help")
            ("groups", boost::program_options::value(&settings.columnCount), "number of groups")
            ("space", boost::program_options::value(&settings.isSpaceBetweenBytes), "set space between groups")
            ("offset", boost::program_options::value(&addressType), "offset format (hex, dec, both, none)")
            ("dump", boost::program_options::value(&settings.isShowDump), "show hexadecimal representation")
            ("char", boost::program_options::value(&settings.placeHolder), "placeholder for non visible symbols")
            ("widechar", boost::program_options::value(&settings.widePlaceHolder), "placeholder for non invisible in wide char")
            ("array", boost::program_options::value(&settings.cArray), "generate c array")


            ("only", boost::program_options::value(&settings.only), "print only visible symbols")
            ("single", boost::program_options::value(&settings.single), "delete first non visible symbols in every group")
            ("compress", boost::program_options::value(&settings.compress), "replace following non visible symbols to one")

            ("newline", boost::program_options::value(&settings.newline), "visible word starts with new line")
            ("length", boost::program_options::value(&settings.length), "min symbols in word for dysplay")

            ("wordwide", boost::program_options::value(&settings.wordwide), "combine wide-char word")
            ;
    try
    {
        boost::program_options::variables_map vm;
        store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("offset"))
            settings.offset = hexdump::GetOffsetType(addressType);

        if (vm.count("help") || argc < 2 || settings.offset == hexdump::Offset::Unknown)
        {
            std::cout << desc << "\n";
            return false;
        }
        fileName = argv[1];
    }

    catch (...)
    {
        std::cout << desc << "\n";
        return false;
    }

    return true;
}