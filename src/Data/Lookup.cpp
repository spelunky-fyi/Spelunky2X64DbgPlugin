#include "Spelunky2.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <QString>
#include <memory>

uintptr_t S2Plugin::Spelunky2::getAfterBundle(uintptr_t sectionStart, size_t sectionSize)
{
    constexpr size_t sevenMegs = 7ull * 1024 * 1024;
    auto Spelunky2AfterBundle = Script::Pattern::FindMem(sectionStart + sectionSize - sevenMegs, sevenMegs, "55 41 57 41 56 41 55 41 54");
    if (Spelunky2AfterBundle == 0)
        displayError("Lookup error: unable to find 'after_bundle' location");

    return Spelunky2AfterBundle;
}

uintptr_t S2Plugin::Spelunky2::get_GameManagerPtr(bool quiet)
{
    if (mGameManagerPtr != 0)
        return mGameManagerPtr;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "C6 80 39 01 00 00 00 48");
    if (instructionOffset == 0)
    {
        if (!quiet)
            displayError("Lookup error: unable to find GameManager");

        return 0;
    }

    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 10);
    auto offsetPtr = instructionOffset + pcOffset + 14;
    mGameManagerPtr = Script::Memory::ReadQword(offsetPtr);
    if (!Script::Memory::IsValidPtr(mGameManagerPtr))
    {
        if (!quiet)
            displayError("Lookup error: GameManager not yet initialised");

        mGameManagerPtr = 0;
    }
    return mGameManagerPtr;
}

const S2Plugin::EntityDB& S2Plugin::Spelunky2::get_EntityDB()
{
    if (mEntityDB.ptr != 0)
        return mEntityDB;

    auto instructionEntitiesPtr = Script::Pattern::FindMem(afterBundle, afterBundleSize, "A4 84 E4 CA DA BF 4E 83");
    if (instructionEntitiesPtr == 0)
    {
        displayError("Lookup error: unable to find EntityDB");
        return mEntityDB;
    }

    auto entitiesPtr = instructionEntitiesPtr - 33 + 7 + (duint)Script::Memory::ReadDword(instructionEntitiesPtr - 30);
    mEntityDB.ptr = Script::Memory::ReadQword(entitiesPtr);
    if (!Script::Memory::IsValidPtr(mEntityDB.ptr))
    {
        displayError("Lookup error: EntityDB not yet initialised");
        mEntityDB.ptr = 0;
    }

    return mEntityDB;
}

const S2Plugin::TextureDB& S2Plugin::Spelunky2::get_TextureDB()
{
    if (mTextureDB.ptr != 0)
        return mTextureDB;

    auto instructionPtr = Script::Pattern::FindMem(afterBundle, afterBundleSize, "4C 89 C6 41 89 CF 8B 1D");
    if (instructionPtr == 0)
    {
        displayError("Lookup error: unable to find TextureDB");
        return mTextureDB;
    }

    auto textureStartAddress = instructionPtr + 12 + (duint)Script::Memory::ReadDword(instructionPtr + 8);
    auto textureCount = Script::Memory::ReadQword(textureStartAddress);
    if (textureCount == 0)
    {
        displayError("Lookup error: TextureDB not yet initialised");
        return mTextureDB;
    }
    mTextureDB.ptr = textureStartAddress + 0x8;

    constexpr uintptr_t textureSize = 0x40ull;
    for (auto x = 0; x < (std::min)(500ull, textureCount); ++x)
    {
        uintptr_t offset = mTextureDB.ptr + textureSize * x;
        auto textureID = Script::Memory::ReadQword(offset);
        mTextureDB.mHighestID = std::max(mTextureDB.mHighestID, textureID);

        auto nameOffset = offset + 0x8;

        size_t value = (nameOffset == 0 ? 0 : Script::Memory::ReadQword(Script::Memory::ReadQword(nameOffset)));
        if (value != 0)
        {
            std::string name = ReadConstString(value);

            mTextureDB.mTextureNamesStringList << QString("Texture %1 (%2)").arg(textureID).arg(QString::fromStdString(name));
            mTextureDB.mTextures.emplace(textureID, std::make_pair(std::move(name), offset));
        }
    }
    return mTextureDB;
}

uintptr_t S2Plugin::Spelunky2::get_OnlinePtr()
{
    if (mOnlinePtr != 0)
        return mOnlinePtr;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8B 05 ?? ?? ?? ?? 80 B8 00 02 00 00 FF");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find Online");
        return mOnlinePtr;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mOnlinePtr = Script::Memory::ReadQword(instructionOffset + 7 + relativeOffset);
    if (!Script::Memory::IsValidPtr(mOnlinePtr))
    {
        displayError("Lookup error: Online not yet initialised");
        mOnlinePtr = 0;
    }

    return mOnlinePtr;
}

const S2Plugin::ParticleDB& S2Plugin::Spelunky2::get_ParticleDB()
{
    if (mParticleDB.ptr != 0)
        return mParticleDB;

    // Spelunky 1.20.4d, 1.23.1b: last id = 0xDB 219
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "FE FF FF FF 66 C7 05");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find ParticleDB (1)");
        return mParticleDB;
    }
    mParticleDB.ptr = instructionOffset + 13 + (duint)Script::Memory::ReadDword(instructionOffset + 7);
    if (!Script::Memory::IsValidPtr(mParticleDB.ptr))
    {
        displayError("Lookup error: unable to find ParticleDB (2)");
        mParticleDB.ptr = 0;
    }
    return mParticleDB;
}

const S2Plugin::CharacterDB& S2Plugin::Spelunky2::get_CharacterDB()
{
    if (mCharacterDB.ptr != 0)
        return mCharacterDB;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 6B C3 2C 48 8D 15 ?? ?? ?? ?? 48");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find CharacterDB (1)");
        return mCharacterDB;
    }
    mCharacterDB.ptr = instructionOffset + 11 + (duint)Script::Memory::ReadDword(instructionOffset + 7);
    if (!Script::Memory::IsValidPtr(mCharacterDB.ptr))
    {
        displayError("Lookup error: unable to find CharacterDB (2)");
        mCharacterDB.ptr = 0;
        return mCharacterDB;
    }

    const size_t characterSize = mCharacterDB.characterSize();
    auto& stringsTable = get_StringsTable();
    if (stringsTable.isValid())
    {
        for (size_t x = 0; x < 20; ++x)
        {
            size_t startOffset = mCharacterDB.ptr + (x * characterSize);
            size_t offset = startOffset;
            QString characterName = stringsTable.stringForIndex(Script::Memory::ReadDword(offset + 0x14));

            mCharacterDB.mCharacterNamesStringList << characterName;
        }
    }
    return mCharacterDB;
}

const S2Plugin::StringsTable& S2Plugin::Spelunky2::get_StringsTable()
{
    if (mStringsTable.ptr != 0)
        return mStringsTable;

    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8D 15 ?? ?? ?? ?? 4C 8B 0C CA");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find StringsTable");
        return mStringsTable;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionOffset + 3);
    auto addr = instructionOffset + 7 + relativeOffset;
    if (Script::Memory::ReadQword(addr) == 0)
    {
        displayError("Lookup error: StringsTable not yet initialised");
        return mStringsTable;
    }

    mStringsTable.ptr = addr;
    return mStringsTable;
}

const S2Plugin::VirtualTableLookup& S2Plugin::Spelunky2::get_VirtualTableLookup()
{
    if (mVirtualTableLookup.mTableStartAddress != 0)
        return mVirtualTableLookup;

    size_t gsAmountOfPointers = mVirtualTableLookup.count();
    mVirtualTableLookup.mOffsetToTableEntries.reserve(gsAmountOfPointers);

    // From 1.23.2 on, the base isn't on D3Dcompile any more, so just look up the first pointer by pattern
    auto instructionOffset = Script::Pattern::FindMem(afterBundle, afterBundleSize, "48 8D 0D ?? ?? ?? ?? 48 89 0D ?? ?? ?? ?? 48 C7 05");
    if (instructionOffset == 0)
    {
        displayError("Lookup error: unable to find VirtualTable start (1)");
        return mVirtualTableLookup;
    }

    auto pcOffset = Script::Memory::ReadDword(instructionOffset + 3);
    mVirtualTableLookup.mTableStartAddress = instructionOffset + pcOffset + 7;
    if (!Script::Memory::IsValidPtr(mVirtualTableLookup.mTableStartAddress))
    {
        displayError("Lookup error: unable to find VirtualTable start (2)");
        mVirtualTableLookup.mTableStartAddress = 0;
        return mVirtualTableLookup;
    }

    // import the pointers
    auto buffer = std::make_unique<size_t[]>(gsAmountOfPointers);

    Script::Memory::Read(mVirtualTableLookup.mTableStartAddress, buffer.get(), gsAmountOfPointers * sizeof(size_t), nullptr);
    for (size_t x = 0; x < gsAmountOfPointers; ++x)
    {
        size_t pointer = buffer[x];
        VirtualTableEntry e;
        e.isValidAddress = Script::Memory::IsValidPtr(pointer);
        e.offset = x;
        e.value = pointer;
        mVirtualTableLookup.mOffsetToTableEntries.emplace(x, std::move(e));
    }
    return mVirtualTableLookup;
}

uintptr_t S2Plugin::Spelunky2::get_GameAPIPtr()
{
    if (mGameAPIPtr != 0)
        return mGameAPIPtr;

    auto instructionAddress = Script::Pattern::FindMem(afterBundle, afterBundleSize, "C6 00 00 48 C7 40 18 00 00 00 00");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find GameAPI (1)");
        return mGameAPIPtr;
    }
    instructionAddress = Script::Pattern::FindMem(instructionAddress - 0x30, 0x30, "48 8B 05");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find GameAPI (2)");
        return mGameAPIPtr;
    }

    auto relativeOffset = Script::Memory::ReadDword(instructionAddress + 3);
    auto gameAPIPointer = instructionAddress + 7 + relativeOffset;
    mGameAPIPtr = Script::Memory::ReadQword(gameAPIPointer);
    if (!Script::Memory::IsValidPtr(mGameAPIPtr))
    {
        displayError("Lookup error: GameAPI not yet initialized");
        mGameAPIPtr = 0;
    }
    return mGameAPIPtr;
}

uintptr_t S2Plugin::Spelunky2::get_HudPtr()
{
    if (mHudPtr != 0)
        return mHudPtr;

    auto instructionAddress = Script::Pattern::FindMem(afterBundle, afterBundleSize, "41 C6 47 6B 01");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find Hud (1)");
        return mHudPtr;
    }
    instructionAddress = Script::Pattern::FindMem(instructionAddress + 5, 0x24, "48 8D 0D");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find Hud (2)");
        return mHudPtr;
    }

    auto relativeOffset = Script::Memory::ReadDword(instructionAddress + 3);
    mHudPtr = instructionAddress + 7 + relativeOffset;
    if (!Script::Memory::IsValidPtr(mHudPtr))
    {
        displayError("Lookup error: unable to find Hud (3)");
        mHudPtr = 0;
    }
    return mHudPtr;
}

uintptr_t S2Plugin::Spelunky2::get_SaveStatesPtr()
{
    if (mSaveStatesPtr != 0)
        return mSaveStatesPtr;

    auto instructionAddress = Script::Pattern::FindMem(afterBundle, afterBundleSize, "90 83 C1 FF");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find SaveStates (1)");
        return mSaveStatesPtr;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionAddress + 6);
    mSaveStatesPtr = instructionAddress + 10 + relativeOffset;
    if (!Script::Memory::IsValidPtr(mSaveStatesPtr))
    {
        displayError("Lookup error: unable to find SaveStates (2)");
        mSaveStatesPtr = 0;
    }
    return mSaveStatesPtr;
}

uintptr_t S2Plugin::Spelunky2::get_DebugSettingsPtr()
{
    if (mDebugSettingsPtr != 0)
        return mDebugSettingsPtr;

    auto instructionAddress = Script::Pattern::FindMem(afterBundle, afterBundleSize, "F3 41 0F 11 47 38 8B 05");
    if (instructionAddress == 0)
    {
        displayError("Lookup error: unable to find DebugSettings (1)");
        return mDebugSettingsPtr;
    }
    auto relativeOffset = Script::Memory::ReadDword(instructionAddress + 8);
    mDebugSettingsPtr = instructionAddress + 12 + relativeOffset;
    if (!Script::Memory::IsValidPtr(mDebugSettingsPtr))
    {
        displayError("Lookup error: unable to find DebugSettings (2)");
        mDebugSettingsPtr = 0;
    }

    return mDebugSettingsPtr;
}
