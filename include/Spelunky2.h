#pragma once

#include "Data/CharacterDB.h"
#include "Data/EntityDB.h"
#include "Data/ParticleDB.h"
#include "Data/StringsTable.h"
#include "Data/TextureDB.h"
#include "Data/VirtualTableLookup.h"
#include <QString>
#include <cstdint>

namespace S2Plugin
{
    constexpr uint32_t TEB_offset = 0x120;

    struct Spelunky2
    {
        static Spelunky2* get();
        static void reset();
        static bool is_loaded()
        {
            return get() != nullptr;
        };

        //
        uintptr_t get_GameManagerPtr(bool quiet = false);
        uintptr_t get_DebugSettingsPtr();
        uintptr_t get_SaveDataPtr(bool quiet);
        uintptr_t get_OnlinePtr();
        uintptr_t get_GameAPIPtr();
        uintptr_t get_HudPtr();
        uintptr_t get_SaveStatesPtr();
        uintptr_t get_StatePtr(bool quiet)
        {
            if (auto base = get_HeapBase(quiet); base != 0)
                return base + GAME_OFFSET::STATE;

            return 0;
        };
        uintptr_t get_LevelGenPtr(bool quiet)
        {

            if (auto base = get_HeapBase(quiet); base != 0)
                return base + GAME_OFFSET::LEVEL_GEN;

            return 0;
        };
        uintptr_t get_LiquidEnginePtr(bool quiet)
        {
            if (auto base = get_HeapBase(quiet); base != 0)
                return base + GAME_OFFSET::LIQUID_ENGINE;

            return 0;
        }
        uintptr_t get_HeapBase(bool quiet);

        TextureDB& get_TextureDB();
        const CharacterDB& get_CharacterDB();
        const ParticleDB& get_ParticleDB();
        const EntityDB& get_EntityDB();
        const StringsTable& get_StringsTable();
        const VirtualTableLookup& get_VirtualTableLookup();
        //

        uintptr_t find(const char* pattern, uintptr_t start = 0, size_t size = 0) const;
        uintptr_t find_between(const char* pattern, uintptr_t start = 0, uintptr_t end = 0) const;

        const QString& themeNameOfOffset(uintptr_t offset);

      private:
        static Spelunky2* ptr;

        uintptr_t codeSectionStart{0};
        size_t codeSectionSize{0};
        uintptr_t afterBundle{0};
        size_t afterBundleSize{0};
        uintptr_t heapBasePtr{0};

        uintptr_t mGameManagerPtr{0};
        uintptr_t mOnlinePtr{0};
        uintptr_t mGameAPIPtr{0};
        uintptr_t mHudPtr{0};
        uintptr_t mSaveStatesPtr{0};
        uintptr_t mDebugSettingsPtr{0};

        EntityDB mEntityDB;
        ParticleDB mParticleDB;
        TextureDB mTextureDB;
        CharacterDB mCharacterDB;
        StringsTable mStringsTable;
        VirtualTableLookup mVirtualTableLookup;

        static uintptr_t getAfterBundle(uintptr_t sectionStart, size_t sectionSize);

        Spelunky2() = default;
        ~Spelunky2(){};
        Spelunky2(const Spelunky2&) = delete;
        Spelunky2& operator=(const Spelunky2&) = delete;

        enum GAME_OFFSET : size_t
        {
            UNKNOWN1 = 0x8,            // - ?
            MALLOC = 0x20,             // - malloc base
            ILLUMINATION_SYNC = 0x3D0, // - illumination sync timer
            PRNG = 0x3F0,              // - PRNG
            STATE = 0x4A0,             // - State Memory
            LEVEL_GEN = 0xD7B30,       // - level gen
            LIQUID_ENGINE = 0xD8650,   // - liquid physics
            UNKNOWN3 = 0x108420,       // - some vector?
        };
        friend class ViewSaveStates;
    };
} // namespace S2Plugin
