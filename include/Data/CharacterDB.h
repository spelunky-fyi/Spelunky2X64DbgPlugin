#pragma once

#include <QStringList>
#include <cstdint>

namespace S2Plugin
{
    class CharacterDB
    {
      public:
        static constexpr uint8_t charactersCount() noexcept
        {
            // [Known Issue]: constant value, needs to rebuild program to update
            return 20;
        }
        const QStringList& characterNamesStringList() const noexcept
        {
            return mCharacterNamesStringList;
        }
        bool isValid() const
        {
            return ptr != 0;
        }
        uintptr_t offsetFromIndex(uint8_t id) const
        {
            return ptr + id * characterSize();
        }
        static constexpr size_t characterSize()
        {
            // [Known Issue]: constant value, needs to rebuild program to update
            return 0x2Cu;
        }

      private:
        uintptr_t ptr{0};
        QStringList mCharacterNamesStringList;

        CharacterDB() = default;
        ~CharacterDB(){};
        CharacterDB(const CharacterDB&) = delete;
        CharacterDB& operator=(const CharacterDB&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
