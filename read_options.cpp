#include "read_options.h"

namespace command_line
{

bool ReadOptions(int argc, char** argv, std::string& fileName, dump::DumperSettings& settings)
{
    namespace po = boost::program_options;
    boost::program_options::options_description desc("Options");
    std::string addressType;
    size_t bufferSize = 0;
    desc.add_options()
        ("help",      "help")
        ("begin",     po::value(&settings.range.begin),              "begin")
        ("end",       po::value(&settings.range.end),                "end")
        ("size",      po::value(&bufferSize),                        "count of bytes from begin")
        ("group",     po::value(&settings.columnCount),              "number of groups")
        ("bytes",     po::value(&settings.bytesInGroup),             "number of bytes in one group")
        ("offset",    po::value(&addressType),                       "offset format (hex, dec, both, none)")
        ("detail",    po::value(&settings.showDetailed),             "detailed format for offset (actual for --shift=true)")
        ("dump",      po::value(&settings.isShowDump),               "show dump column")
        ("asc",       po::value(&settings.isShowAscii),              "show ascii column")
        ("debug",     po::value(&settings.isShowDebug),              "show debug information")
        ("space",     po::value(&settings.isSpaceBetweenAsciiBytes), "set space between ascii groups")
        ("spacedump", po::value(&settings.isSpaceBetweenDumpBytes),  "set space between dump groups")
        ("char",      po::value(&settings.placeHolder),              "placeholder for non visible symbols")
        ("zero",      po::value(&settings.zeroPlaceHolder),          "placeholder for zero")
        ("widechar",  po::value(&settings.widePlaceHolder),          "placeholder for invisible symbols in wide-char words")
        ("wide",      po::value(&settings.useWideChar),              "use wide-char mode for words")
        ("array",     po::value(&settings.isCArray),                 "generate c-array (0x00, ...)")
        ("ladder",    po::value(&settings.ladder),                   "use intend for separate words")
        ("newline",   po::value(&settings.newLine),                  "every visible word starts with new line")
        ("shift",     po::value(&settings.useRelativeAddress),       "choose true for use --begin as offset=0")
        ("key",       po::value(&settings.key),                      "find key (string value)")
        ("hkey",      po::value<StringVector>(&settings.hkeyValueVector)->multitoken(), "find key (hex values)")
        ("from",      po::value(&settings.hkeyFrom),                  "begin from first key (hex value)")
        ("till",      po::value(&settings.hkeyTill),                  "finish with last key (hex value)")
        ("skip",      po::value(&settings.skipTextWithoutKeys),      "skip lines where key is absent")
        ("empty",     po::value(&settings.isShowEmptyLines),         "show empty lines (as ...)")
        ("before",    po::value(&settings.countBytesBeforeKey),      "count lines before key (for skip mode)")
        ("after",     po::value(&settings.countBytesAfterKey),       "count lines after key (for skip mode)")
        ("range",     po::value(&settings.countBytesAfterHkeyFrom),  "count bytes after --from key")
        ("grey",      po::value(&settings.printZeroAsGrey),          "print 0x00 in grey color")
        ;
    try
    {
        boost::program_options::variables_map vm;
        store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        notify(vm);

        if (vm.count("offset"))
            settings.offset = dump::GetOffsetType(addressType);

        if (vm.count("help") || argc < 2 || settings.offset == dump::OffsetTypes::Unknown)
        {
            std::cout << desc << "\n";
            return false;
        }
        settings.isShowOffset = settings.offset != dump::OffsetTypes::None;

        if (!vm.count("end"))
            settings.range.end = vm.count("size") ? settings.range.begin + bufferSize - 1
                                                  : std::numeric_limits<size_t>::max();

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

} // namespace command_line
