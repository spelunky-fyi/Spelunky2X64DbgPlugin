#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <string>

namespace S2Plugin
{
    template <typename T>
    [[nodiscard]] inline T Read(uintptr_t addr)
    {
        if constexpr (sizeof(T) == 1)
            return static_cast<T>(Script::Memory::ReadByte(addr));
        else if constexpr (sizeof(T) == 2)
            return static_cast<T>(Script::Memory::ReadWord(addr));
        else if constexpr (sizeof(T) == 4)
            return static_cast<T>(Script::Memory::ReadDword(addr));
        else if constexpr (sizeof(T) == 8)
            return static_cast<T>(Script::Memory::ReadQword(addr));
        else
        {
            T x{};
            Script::Memory::Read(addr, &x, sizeof(T), nullptr);
            return x;
        }
    }

    template <typename T>
    [[nodiscard]] std::basic_string<T> ReadConstBasicString(uintptr_t addr)
    {
        if (addr == 0)
            return {};
        // reads thru the characters twice but avoids static buffers or resizing string by adding characters one by one
        // which is apparently what the basic string constructor does when constructing from const char* pointer
        constexpr auto char_size = sizeof(T);

        size_t size = 0;
        T c = Read<T>(addr);
        while (c != 0)
        {
            size++;
            c = Read<T>(addr + (size * char_size));
        }
        std::basic_string<T> str;
        str.resize(size);
        size_t read_size = 0;
        Script::Memory::Read(addr, str.data(), size * char_size, &read_size);
        if (size * char_size != read_size)
            dprintf("[ReadConstBasicString] read (bytes): %d expected: %d\n", read_size, size * char_size);
        return str;
    }
    [[nodiscard]] inline std::string ReadConstString(uintptr_t addr)
    {
        return ReadConstBasicString<char>(addr);
    }
} // namespace S2Plugin
