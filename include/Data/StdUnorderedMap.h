#pragma once

#include "pluginmain.h"
#include <cstdint>
#include <utility>

namespace S2Plugin
{
    constexpr size_t gsStdUnorderedMapSize = 64;
    // std::unordered_map internally basically consists of a std::list (but with next/prev switched around) and std::vector + hash stuff
    class StdUnorderedMap
    {
      public:
        StdUnorderedMap(uintptr_t address, size_t keySize, uint8_t alignment) : _end(0, 0)
        {
            size_t offset = keySize + sizeof(uintptr_t) * 2;

            // dealing with the padding between key and value
            switch (alignment)
            {
                case 0:
                case 1:
                    break;
                case 2:
                    offset = (offset + 1) & ~1;
                    break;
                case 3:
                case 4:
                    offset = (offset + 3) & ~3;
                    break;
                case 5:
                case 6:
                case 7:
                case 8:
                default:
                    offset = (offset + 7) & ~7;
                    break;
            }
            uintptr_t data[2];
            Script::Memory::Read(address + sizeof(uintptr_t), &data, sizeof(data), nullptr);
            _end.mNodeAddress = data[0];
            _end.mValueOffset = offset;
            mSize = data[1];
        };

        struct Node
        {
            Node(uintptr_t address, size_t valueOffset) : mNodeAddress(address), mValueOffset(valueOffset){};

            uintptr_t value_ptr() const noexcept
            {
                return mNodeAddress + mValueOffset;
            }
            uintptr_t key_ptr() const noexcept
            {
                return mNodeAddress + sizeof(uintptr_t) * 2;
            }
            std::pair<uintptr_t, uintptr_t> operator*() const noexcept
            {
                return std::pair<uintptr_t, uintptr_t>(key_ptr(), value_ptr());
            }
            uintptr_t valueOffset() const noexcept
            {
                return mValueOffset;
            }
            Node next() const
            {
                return Node(Script::Memory::ReadQword(mNodeAddress), mValueOffset);
            }
            Node prev() const
            {
                return Node(Script::Memory::ReadQword(mNodeAddress + sizeof(uintptr_t)), mValueOffset);
            }
            Node& operator++()
            {
                mNodeAddress = Script::Memory::ReadQword(mNodeAddress);
                return *this;
            }
            Node& operator--()
            {
                mNodeAddress = Script::Memory::ReadQword(mNodeAddress + sizeof(uintptr_t));
                return *this;
            }
            bool operator==(const Node& other) const
            {
                return mNodeAddress == other.mNodeAddress;
            }
            bool operator!=(const Node& other) const
            {
                return mNodeAddress != other.mNodeAddress;
            }

          private:
            uintptr_t mNodeAddress;
            size_t mValueOffset;
            friend class StdUnorderedMap;
        };

        bool empty() const noexcept
        {
            return mSize == 0;
        }
        size_t size() const noexcept
        {
            return mSize;
        }
        Node begin() const
        {
            return _end.next();
        }
        Node end() const noexcept
        {
            return _end;
        }
        Node cbegin() const
        {
            return _end.next();
        }
        Node cend() const noexcept
        {
            return _end;
        }

      private:
        Node _end;
        size_t mSize;
    };
} // namespace S2Plugin
