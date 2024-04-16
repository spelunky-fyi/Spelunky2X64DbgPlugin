#include "Data/StringsTable.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QString>

QString S2Plugin::StringsTable::stringForIndex(uint32_t idx) const
{
    if (count() < idx)
    {
        return QString("INVALID OR NOT APPLICABLE");
    }
    auto str = ReadConstBasicString<ushort>(stringAddressOfIndex(idx));
    return QString::fromUtf16(str.c_str(), static_cast<int>(str.size()));
}

uintptr_t S2Plugin::StringsTable::stringAddressOfIndex(uint32_t idx) const
{
    return Script::Memory::ReadQword(addressOfIndex(idx));
}
