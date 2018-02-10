#include "dumper_settings.h"

namespace dump
{

OffsetTypes GetOffsetType(const std::string& offset)
{
    if (offset == "hex")
        return OffsetTypes::Hex;
    if (offset == "dec")
        return OffsetTypes::Dec;
    if (offset == "both" || offset == "true")
        return OffsetTypes::Both;
    if (offset == "none" || offset == "false")
        return OffsetTypes::None;

    return OffsetTypes::Unknown;
}

} // namespace dump
