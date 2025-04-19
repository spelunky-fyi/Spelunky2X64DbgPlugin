#include "Spelunky2.h"

#include "Configuration.h"
#include "Data/EntityDB.h"
#include "pluginmain.h"
#include <QIcon>
#include <QMessageBox>

S2Plugin::Spelunky2* S2Plugin::Spelunky2::ptr = nullptr;

S2Plugin::Spelunky2* S2Plugin::Spelunky2::get()
{
    if (ptr == nullptr)
    {
        // see if we can get the main module
        Script::Module::ModuleInfo moduleInfo;
        if (!Script::Module::GetMainModuleInfo(&moduleInfo))
        {
            displayError("GetMainModuleInfo failed; Make sure spel2.exe is loaded");
            return nullptr;
        }
        // see if the main module is Spelunky 2
        if (std::string{"spel2.exe"}.compare(moduleInfo.name) != 0)
        {
            displayError("Main module is not spel2.exe");
            return nullptr;
        }

        // retrieve the memory map and loop every entry, until we find the .text section of spel2.exe
        uintptr_t Spelunky2CodeSectionStart{0};
        size_t Spelunky2CodeSectionSize{0};
        uintptr_t Spelunky2AfterBundle{0};

        MEMMAP memoryMap = {0};
        DbgMemMap(&memoryMap);
        for (auto i = 0; i < memoryMap.count; ++i)
        {
            MEMORY_BASIC_INFORMATION mbi = (memoryMap.page)[i].mbi;
            auto info = std::string((memoryMap.page)[i].info);
            uintptr_t baseAddress = (uintptr_t)mbi.BaseAddress;

            char name[MAX_MODULE_SIZE + 1] = {0};
            Script::Module::NameFromAddr(baseAddress, name);
            if (std::string{"spel2.exe"}.compare(name) != 0 || info.find(".text") == std::string::npos)
                continue;

            Spelunky2CodeSectionStart = baseAddress;
            Spelunky2CodeSectionSize = mbi.RegionSize;
            break;
        }

        if (Spelunky2CodeSectionStart == 0 && Spelunky2CodeSectionSize == 0)
        {
            displayError("Could not locate the .text section in the loaded spel2.exe image");
            return false;
        }

        Spelunky2AfterBundle = getAfterBundle(Spelunky2CodeSectionStart, Spelunky2CodeSectionSize);
        if (Spelunky2AfterBundle == 0)
            return false;

        auto addr = new Spelunky2{};
        addr->codeSectionStart = Spelunky2CodeSectionStart;
        addr->codeSectionSize = Spelunky2CodeSectionSize;
        addr->afterBundle = Spelunky2AfterBundle;
        addr->afterBundleSize = Spelunky2CodeSectionStart + Spelunky2CodeSectionSize - Spelunky2AfterBundle;
        ptr = addr;
    }
    return ptr;
}

void S2Plugin::Spelunky2::reset()
{
    if (ptr != nullptr)
    {
        delete ptr;
        ptr = nullptr;
    }
}

uintptr_t S2Plugin::Spelunky2::find(const char* pattern, uintptr_t start, size_t size) const
{
    if (start == 0)
        start = afterBundle;

    if (size == 0)
        size = afterBundleSize - (start == 0 ? 0 : start - afterBundle);

    return Script::Pattern::FindMem(start, size, pattern);
}

uintptr_t S2Plugin::Spelunky2::find_between(const char* pattern, uintptr_t start, uintptr_t end) const
{
    if (start == 0)
        start = afterBundle;

    size_t size;
    if (end == 0)
        size = afterBundleSize - (start == 0 ? 0 : start - afterBundle);
    else
        size = end - start;

    return Script::Pattern::FindMem(start, size, pattern);
}

uintptr_t S2Plugin::Spelunky2::get_SaveDataPtr(bool quiet)
{
    auto gm = get_GameManagerPtr();
    if (gm == 0)
        return 0;

    auto heapOffsetSaveGame = Script::Memory::ReadQword(Script::Memory::ReadQword(gm + 8));
    if (heapOffsetSaveGame == 0)
    {
        if (!quiet)
            displayError("Lookup error: found GameManager but the SaveData offset is 0 (too soon?)");

        return 0;
    }

    auto heapBase = get_HeapBase(quiet);
    return heapBase == NULL ? 0 : heapBase + heapOffsetSaveGame;
}

const QString& S2Plugin::Spelunky2::themeNameOfOffset(uintptr_t offset)
{
    auto config = Configuration::get();
    auto levelGenPtr = get_LevelGenPtr(true);
    uintptr_t firstThemeOffset = config->offsetForField(MemoryFieldType::LevelGen, "theme_dwelling", levelGenPtr);

    static const QStringList themeNames = {
        "DWELLING",     "JUNGLE",       "VOLCANA", "OLMEC", "TIDE POOL", "TEMPLE",         "ICE CAVES", "NEO BABYLON", "SUNKEN CITY",
        "COSMIC OCEAN", "CITY OF GOLD", "DUAT",    "ABZU",  "TIAMAT",    "EGGPLANT WORLD", "HUNDUN",    "BASE CAMP",   "ARENA",
    };
    if (levelGenPtr != 0)
    {
        for (uint8_t idx = 0; idx < themeNames.size(); ++idx)
        {
            uintptr_t testPtr = Script::Memory::ReadQword(firstThemeOffset + idx * 0x8ull);
            if (testPtr == offset)
                return themeNames.at(idx);
        }
    }
    static const QString unknown{"UNKNOWN THEME"};
    return unknown;
}

uintptr_t S2Plugin::Spelunky2::get_HeapBase(bool quiet)
{
    if (heapBasePtr == 0)
    {
        THREADLIST threadList;
        uintptr_t heapBase{0};
        uintptr_t heapBasePtrTemp{0};
        DbgGetThreadList(&threadList);
        if (threadList.count == 0)
        {
            if (!quiet)
                displayError("Could not retrieve thread list\nYou might be too fast, wait for the info in bottom left corner to change to \"Running\"");

            return NULL;
        }
        for (auto x = 0; x < threadList.count; ++x)
        {
            auto threadAllInfo = threadList.list[x];
            if (threadAllInfo.BasicInfo.ThreadNumber == 0) // main thread
            {
                auto tebAddress = DbgGetTebAddress(threadAllInfo.BasicInfo.ThreadId);
                auto tebAddress11Ptr = Script::Memory::ReadQword(tebAddress + (11 * sizeof(uintptr_t)));
                auto tebAddress11Value = Script::Memory::ReadQword(tebAddress11Ptr);
                heapBasePtrTemp = tebAddress11Value + TEB_offset;
                heapBase = Script::Memory::ReadQword(heapBasePtrTemp);
                break;
            }
        }
        if (!Script::Memory::IsValidPtr(heapBasePtrTemp) || !Script::Memory::IsValidPtr(heapBase))
        {
            if (!quiet)
                displayError("Could not retrieve heap base of the main thread!\nYou might be too fast, wait for the info in bottom left corner to change to \"Running\"");

            return NULL;
        }
        heapBasePtr = heapBasePtrTemp;
    }
    return Script::Memory::ReadQword(heapBasePtr);
};

static constexpr uint32_t lowbias32(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352d;
    x ^= x >> 15;
    x *= 0x846ca68b;
    x ^= x >> 16;
    return x;
}

// Just for refrence
// struct RobinHoodTableEntry
// {
//    uint32_t uid_plus_one;
//    uint32_t padding;
//    Entity* entity;
// };

uintptr_t S2Plugin::Spelunky2::findEntitybyUID(uint32_t uid, uintptr_t statePtr)
{
    if (statePtr == 0)
        statePtr = get_StatePtr(true);
    // ported from overlunky
    if (uid == ~0 || statePtr == 0)
        return 0;

    // [Known Issue]: Static value, have to restart programm for size to update
    static size_t mask_offset = Configuration::get()->offsetForField(MemoryFieldType::State, "uid_to_entity_mask");

    const uint32_t mask = Script::Memory::ReadDword(statePtr + mask_offset);
    const uint32_t target_uid_plus_one = lowbias32(uid + 1u);
    uint32_t cur_index = target_uid_plus_one & mask;
    const uintptr_t uid_to_entity_data = Script::Memory::ReadQword(statePtr + mask_offset + 0x8u);

    auto getEntry = [uid_to_entity_data](size_t index)
    {
        constexpr size_t robinHoodTableEntrySize = 0x10u;
        return uid_to_entity_data + index * robinHoodTableEntrySize;
    };

    while (true)
    {
        auto entry = getEntry(cur_index);
        auto uid_plus_one = Script::Memory::ReadDword(entry);
        if (uid_plus_one == target_uid_plus_one)
            return Script::Memory::ReadQword(entry + 0x8u);

        if (uid_plus_one == 0)
            return 0;

        if (((cur_index - target_uid_plus_one) & mask) > ((cur_index - uid_plus_one) & mask))
            return 0;

        cur_index = (cur_index + 1u) & mask;
    }
}
