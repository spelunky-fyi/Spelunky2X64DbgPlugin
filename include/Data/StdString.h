#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <string>

namespace S2Plugin
{
    template <typename T>
    struct StdBasicString
    {
        explicit StdBasicString(size_t addr) : addr(addr){};
        size_t size() const
        {
            return Script::Memory::ReadQword(addr + 0x10);
        }
        size_t length() const
        {
            return size();
        }
        size_t capacity() const
        {
            return Script::Memory::ReadQword(addr + 0x18);
        }
        size_t begin() const
        {
            return string_ptr();
        }
        size_t end() const
        {
            return string_ptr() + length() * sizeof(T);
        }
        bool empty() const
        {
            return length() == 0;
        }
        size_t string_ptr() const
        {
            // test if string is in SSO mode (Short String Optimization)
            // note: this is implementation specific, for std::string MSVC the capacity will be 15, for clang it might be as high as 22
            if (capacity() > std::basic_string<T>{}.capacity())
                return Script::Memory::ReadQword(addr);

            return addr;
        }
        std::basic_string<T> get_string() const
        {
            size_t string_addr = string_ptr();
            size_t string_length = length();
            std::basic_string<T> buffer;
            buffer.resize(string_length);
            if (string_length != 0)
            {
                Script::Memory::Read(string_addr, buffer.data(), string_length * sizeof(T), nullptr);
            }
            return buffer;
        }
        bool operator==(const StdBasicString<T> other) const
        {
            if (string_ptr() == other.string_ptr())
                return true;

            auto l = length();
            auto other_l = other.length();
            if (l != other_l)
                return false;

            if (l == 0) // both lengths the same at this point, so both are 0
                return true;

            return get_string() == other.get_string();
        }

      private:
        uintptr_t addr;
    };

    using StdString = StdBasicString<char>;
    using StdWstring = StdBasicString<ushort>;
} // namespace S2Plugin
