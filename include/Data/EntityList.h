#pragma once

#include "Entity.h"
#include "pluginmain.h"
#include "read_helpers.h"
#include <cstdint>
#include <utility>
#include <vector>

namespace S2Plugin
{
    // this shoudl only ever be used as temporary
    class EntityList
    {
      public:
        explicit EntityList(uintptr_t address)
        {
            Script::Memory::Read(address, this, sizeof(EntityList), nullptr);
        };
        uintptr_t entities() const
        {
            return mEntities;
        }
        uintptr_t uids() const
        {
            return mUids;
        }
        uint32_t capacity() const
        {
            return mCap;
        }
        uint32_t size() const
        {
            return mSize;
        }
        struct Iterator
        {
            // Iterator(){};
            explicit Iterator(const EntityList entityList, uint32_t index) noexcept : Iterator(entityList.begin())
            {
                advance(index);
            }
            void advance(int count) noexcept // should probably be int64
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
                return Entity{Script::Memory::ReadQword(addr.first)};
            }
            uint32_t uid() const
            {
                return Script::Memory::ReadDword(addr.second);
            }

          private:
            explicit Iterator(uintptr_t entitiesAddress, uintptr_t uidsAddress) : addr(entitiesAddress, uidsAddress){};
            std::pair<uintptr_t, uintptr_t> addr;
            friend class EntityList;
        };

        Iterator begin() const
        {
            return Iterator{entities(), uids()};
        }
        Iterator end() const
        {
            uintptr_t entitiesEnd = entities() + size() * sizeof(uintptr_t);
            uintptr_t uidsEnd = uids() + size() * sizeof(uint32_t);
            return Iterator{entitiesEnd, uidsEnd};
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
        std::vector<uintptr_t> getAllEntities() const
        {
            std::vector<uintptr_t> result;
            result.resize(size());
            Script::Memory::Read(entities(), result.data(), size() * sizeof(uintptr_t), nullptr);
            return result;
        }
        std::vector<uint32_t> getAllUids() const
        {
            std::vector<uint32_t> result;
            result.resize(size());
            Script::Memory::Read(uids(), result.data(), size() * sizeof(uint32_t), nullptr);
            return result;
        }

      private:
        uintptr_t mEntities{0};
        uintptr_t mUids{0};
        uint32_t mCap{0};
        uint32_t mSize{0};
    };
}; // namespace S2Plugin
