#include "Data/StringsTable.h"

#include "pluginmain.h"
#include "read_helpers.h"
#include <array>

QString S2Plugin::StringsTable::stringForIndex(uint32_t idx) const
{
    if (count() <= idx)
    {
        // [Known Issue]: hardcoded number for valid strings
        if (idx < 1964)
            return QString("StringsDB NOT INITIALISED");

        return QString("INVALID OR NOT APPLICABLE");
    }
    auto str = ReadConstBasicString<ushort>(stringAddressOfIndex(idx));
    return QString::fromUtf16(str.c_str(), static_cast<int>(str.size()));
}

uintptr_t S2Plugin::StringsTable::stringAddressOfIndex(uint32_t idx) const
{
    return Script::Memory::ReadQword(addressOfIndex(idx));
}

uintptr_t S2Plugin::StringsTable::count(bool recount) const
{
    if (size != 0 && !recount)
        return size;

    // read memory in chunks to speed to the process
    std::array<uintptr_t, 200> data{0};
    constexpr size_t expectedMax = 2200;

    for (uint idx = 0; idx < expectedMax / data.size(); ++idx)
    {
        Script::Memory::Read(ptr + idx * data.size() * sizeof(uintptr_t), data.data(), data.size() * sizeof(uintptr_t), nullptr);
        for (uint dataIdx = 0; dataIdx < data.size(); ++dataIdx)
        {
            if (!Script::Memory::IsValidPtr(data[dataIdx]))
            {
                const_cast<StringsTable&>(*this).size = idx * data.size() + dataIdx;
                return size;
            }
        }
    }
    const_cast<StringsTable&>(*this).size = expectedMax;
    return size;
}
