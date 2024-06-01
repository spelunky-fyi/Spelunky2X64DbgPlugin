#pragma once

#include <QStringList>
#include <cstdint>
#include <regex>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class IDNameList
    {
      protected:
        IDNameList(const std::string& relFilePath, const std::regex& regex);

      public:
        uint32_t idForName(const std::string& name) const;
        std::string nameForID(uint32_t id) const;
        uint32_t highestID() const noexcept
        {
            return mHighestID;
        }
        size_t count() const noexcept
        {
            return mEntries.size();
        }
        const QStringList& names() const noexcept
        {
            return mNames;
        }
        bool isValidID(uint32_t id) const
        {
            return (mEntries.find(id) != mEntries.end());
        }
        const std::unordered_map<uint32_t, std::string>& entries() const
        {
            return mEntries;
        }

      private:
        std::unordered_map<uint32_t, std::string> mEntries;
        QStringList mNames;
        uint32_t mHighestID = 0;

        IDNameList() = default;
        IDNameList(const IDNameList&) = delete;
        IDNameList& operator=(const IDNameList&) = delete;
    };
    class ParticleEmittersList : public IDNameList
    {
      public:
        explicit ParticleEmittersList();

        ParticleEmittersList(const ParticleEmittersList&) = delete;
        ParticleEmittersList& operator=(const ParticleEmittersList&) = delete;
    };
    class EntityNamesList : public IDNameList
    {
      public:
        explicit EntityNamesList();

        EntityNamesList(const EntityNamesList&) = delete;
        EntityNamesList& operator=(const EntityNamesList&) = delete;
    };
} // namespace S2Plugin
