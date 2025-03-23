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
            static QStringList characterNames = {
                "Ana Spelunky",      "Margaret Tunnel", "Colin Northward", "Roffy D. Sloth", "Alto Singh",    "Liz Mutton", "Nekka The Eagle",   "LISE Project",
                "Coco Von Diamonds", "Manfred Tunnel",  "Little Jay",      "Tina Flan",      "Valerie Crump", "Au",         "Demi Von Diamonds", "Pilot",
                "Princess Airyn",    "Dirk Yamaoka",    "Guy Spelunky",    "Classic Guy",
            };
            return characterNames;
        }
        bool isValid() const noexcept
        {
            return ptr != 0;
        }
        uintptr_t addressOfIndex(uint8_t id) const noexcept
        {
            return ptr + id * characterSize();
        }
        static constexpr size_t characterSize() noexcept
        {
            // [Known Issue]: constant value, needs to rebuild program to update
            return 0x2Cu;
        }

      private:
        uintptr_t ptr{0};

        CharacterDB() = default;
        ~CharacterDB(){};
        CharacterDB(const CharacterDB&) = delete;
        CharacterDB& operator=(const CharacterDB&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
