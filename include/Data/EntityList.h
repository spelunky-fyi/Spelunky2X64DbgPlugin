#pragma once

#include "Entity.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <cstdint>
#include <utility>

namespace S2Plugin
{

    class EntityList
    {
      public:
        EntityList(uintptr_t _address) : address(_address){};
        uintptr_t entities() const
        {
            return Script::Memory::ReadQword(address);
        }

        uintptr_t uids() const
        {
            return Script::Memory::ReadQword(address + sizeof(uintptr_t));
        }
        uint32_t capacity() const
        {
            return Script::Memory::ReadDword(address + sizeof(uintptr_t) * 2);
        }
        uint32_t size() const
        {
            return Script::Memory::ReadDword(address + sizeof(uintptr_t) * 2 + sizeof(uint32_t));
        }
        struct Iterator
        {
            // Iterator(){};
            Iterator(const EntityList entityList, uint32_t index) noexcept : Iterator(entityList.begin())
            {
                advance(index);
            }
            void advance(int count) noexcept // should probably be int64 ?
            {
                addr.first += count * sizeof(uintptr_t);
                addr.second += count * sizeof(uint32_t);
            }
            Iterator& operator++() noexcept
            {
                advance(1);
                return *this;
            }
            Iterator operator++(int) noexcept
            {
                auto copy = *this;
                advance(1);
                return copy;
            }
            Iterator& operator--() noexcept
            {
                advance(-1);
            }
            Iterator operator--(int) noexcept
            {
                auto copy = *this;
                advance(-1);
                return copy;
            }
            std::pair<uintptr_t, uint32_t> operator*() const
            {
                return {addr.first, Script::Memory::ReadDword(addr.second)};
            }
            bool operator==(const Iterator& other) const noexcept
            {
                return addr.first == other.addr.first;
            }
            bool operator!=(const Iterator& other) const noexcept
            {
                return addr.first != other.addr.first;
            }
            uintptr_t entityRaw() const
            {
                return Script::Memory::ReadQword(addr.first);
            }
            Entity entity() const
            {
                return Script::Memory::ReadQword(addr.first);
            }
            uint32_t uid() const
            {
                return Script::Memory::ReadDword(addr.second);
            }

          private:
            Iterator(uintptr_t entitiesAddress, uintptr_t uidsAddress) : addr(entitiesAddress, uidsAddress){};
            std::pair<uintptr_t, uintptr_t> addr;
            friend class EntityList;
        };

        Iterator begin() const
        {
            uintptr_t pointers[2] = {0, 0};
            // slightly faster then reading both thru ReadQword
            Script::Memory::Read(address, &pointers, sizeof(uintptr_t) * 2, nullptr);
            return {pointers[0], pointers[1]};
        }
        Iterator end() const
        {
            auto full = getFullStruct();
            uintptr_t entitiesEnd = full.entities + full.size * sizeof(uintptr_t);
            uintptr_t uidsEnd = full.uids + full.size * sizeof(uint32_t);
            return {entitiesEnd, uidsEnd};
        }
        const Iterator cbegin() const
        {
            return begin();
        }
        const Iterator cend() const
        {
            return end();
        }
        Iterator find(uint32_t uid) const
        {
            auto endIterator = end();
            for (auto it = begin(); it != endIterator; ++it)
            {
                if (it.uid() == uid)
                    return it;
            }
            return endIterator;
        }
        Iterator findEntity(uintptr_t addr) const
        {
            auto endIterator = end();
            for (auto it = begin(); it != endIterator; ++it)
            {
                if (it.entityRaw() == addr)
                    return it;
            }
            return endIterator;
        }

      private:
        uintptr_t address;
        struct TrueEntityList
        {
            uintptr_t entities{0};
            uintptr_t uids{0};
            uint32_t cap{0};
            uint32_t size{0};

          private:
            TrueEntityList() = delete;
            // TrueEntityList(const TrueEntityList&) = delete;
            // TrueEntityList& operator=(const TrueEntityList&) = delete;
        };
        TrueEntityList getFullStruct() const
        {
            return Read<TrueEntityList>(address);
        }
    };
}; // namespace S2Plugin
