#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <string>

namespace S2Plugin
{
    template <typename T>
    T Read(uintptr_t addr)
    {
        T x{};
        Script::Memory::Read(addr, &x, sizeof(T), nullptr);
        return x;
    }

    template <typename T>
    std::basic_string<T> ReadConstBasicString(uintptr_t addr)
    {
        if (addr == 0)
            return {};
        // reads thru the characters twice but avoids static buffers or resizing string by adding characters one by one
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
    inline std::string ReadConstString(uintptr_t addr)
    {
        return ReadConstBasicString<char>(addr);
    }
} // namespace S2Plugin
