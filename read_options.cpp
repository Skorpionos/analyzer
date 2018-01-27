#include "read_options.h"

bool ReadOptions(int argc, char** argv, std::string& fileName, dump::DumperSettings& settings)
{
    namespace po = boost::program_options;
    boost::program_options::options_description desc("Options");
    std::string addressType;
    desc.add_options()
            ("help", "help")
            ("begin", boost::program_options::value(&settings.begin), "start from offset")
            ("size", boost::program_options::value(&settings.size), "count of bytes to display")
            ("end", boost::program_options::value(&settings.end), "last offset to display")
            ("group", boost::program_options::value(&settings.columnCount), "number of groups")
            ("bytes", boost::program_options::value(&settings.bytesInGroup), "number of bytes in one group")
            ("offset", boost::program_options::value(&addressType), "offset format (hex, dec, both, none)")
            ("detailed", boost::program_options::value(&settings.detailedOffset), "detailed format for offset")
            ("dump", boost::program_options::value(&settings.isShowDump), "show hexadecimal representation")
            ("asc", boost::program_options::value(&settings.isShowAscii), "show ascii representation")
            ("space", boost::program_options::value(&settings.isSpaceBetweenAsciiBytes), "set space between ascii groups")
            ("spacedump", boost::program_options::value(&settings.isSpaceBetweenDumpBytes), "set space between dump groups")
            ("char", boost::program_options::value(&settings.placeHolder), "placeholder for non visible symbols")
            ("zero", boost::program_options::value(&settings.zeroPlaceHolder), "placeholder for zero")
            ("wide", boost::program_options::value(&settings.useWideChar), "combine wide-char word")
            ("widechar", boost::program_options::value(&settings.widePlaceHolder), "placeholder for non invisible in symbols for wide-char words")
            ("array", boost::program_options::value(&settings.IsCArray), "generate c array")
            ("ladder", boost::program_options::value(&settings.ladder), "use intend for separate words")
            ("newline", boost::program_options::value(&settings.newLine), "every visible word starts with new line")
            ("fromstart", boost::program_options::value(&settings.useRelativeAddress), "begin without indent")
            ("key", boost::program_options::value(&settings.key), "key (string value) for found")
            ("hkey", boost::program_options::value(&settings.hkey), "key (hex values) for found")
            ("from", boost::program_options::value(&settings.from), "from key (hex value)")
            ("till", boost::program_options::value(&settings.till), "till key (hex value)")
//            ("length", boost::program_options::value(&settings.length), "min symbols in word for display")
//            ("single", boost::program_options::value(&settings.single), "delete first non visible symbols in every group")
//            ("compress", boost::program_options::value(&settings.compress), "replace following non visible symbols to one")
            ;
    try
    {
        boost::program_options::variables_map vm;
        store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("offset"))
            settings.offset = dump::GetOffsetType(addressType);

        if (vm.count("help") || argc < 2 || settings.offset == dump::Offset::Unknown)
        {
            std::cout << desc << "\n";
            return false;
        }
        settings.isShowOffset = settings.offset != dump::Offset::None;

        if (settings.size !=0  && settings.end == 0)
            settings.end = settings.begin + settings.size - 1;

//        if (!vm.count("zero") )
//            settings.zeroPlaceHolder = settings.placeHolder;

        if (settings.IsCArray)
        {
//            settings.isShowAscii = false;
            // settings.isShowOffset = false;
        }

        if (settings.newLine || settings.ladder)
            settings.isSpaceBetweenDumpBytes = false;

        fileName = argv[1];

        settings.bytesInLine = settings.columnCount * settings.bytesInGroup;
    }

    catch (...)
    {
        std::cout << desc << "\n";
        return false;
    }

    return true;
}