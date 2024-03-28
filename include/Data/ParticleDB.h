#pragma once

#include <cstdint>

namespace S2Plugin
{
    class ParticleDB
    {
      public:
        uintptr_t addressOfIndex(uint32_t particleDBIndex) const
        {
            return ptr == 0ull ? 0ull : ptr + particleDBIndex * particleSize();
        }
        bool isValid() const
        {
            return (ptr != 0);
        }
        static size_t particleSize();

      private:
        uintptr_t ptr{0};

        friend struct Spelunky2;
    };
} // namespace S2Plugin
