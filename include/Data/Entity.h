#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace S2Plugin
{
    constexpr size_t gSmallEntityBucket = 0xD0;
    constexpr size_t gBigEntityBucket = 0x188;

    class Entity
    {
      public:
        explicit Entity(uintptr_t addr) : mEntityPtr(addr){};
        Entity() = delete;

        std::string entityClassName() const;
        uint32_t entityTypeID() const;
        std::string entityTypeName() const;
        static std::vector<std::string> classHierarchy(std::string validClassName);
        std::vector<std::string> classHierarchy() const
        {
            return classHierarchy(entityClassName());
        }

        uint8_t layer() const;
        uint32_t uid() const;
        uintptr_t ptr() const
        {
            return mEntityPtr;
        }
        std::pair<float, float> position() const;
        std::pair<float, float> abs_position() const;

      private:
        uintptr_t mEntityPtr;

        enum ENTITY_OFFSETS
        {
            UID = 0x38,
            LAYER = 0xA0,
            TYPE_PTR = 0x8,
            DB_TYPE_ID = 0x14,
            POS = 0x40,
            OVERLAY = 0x10,
        };
    };
} // namespace S2Plugin
