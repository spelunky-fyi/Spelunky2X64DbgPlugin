#pragma once

#include <QStringList>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace S2Plugin
{
    class TextureDB
    {
      public:
        bool isValid() const
        {
            return (ptr != 0);
        }
        // id != index since there is no id 325 but there is no gap for it
        const std::string& nameForID(uint32_t id) const;
        uintptr_t addressOfID(uint32_t id) const
        {
            if (auto it = mTextures.find(id); it != mTextures.end())
            {
                return it->second.second;
            }
            return 0;
        }
        const QStringList& namesStringList() const noexcept
        {
            return mTextureNamesStringList;
        }
        size_t count() const
        {
            return mTextures.size();
        }
        bool isValidID(uint32_t id) const
        {
            return mTextures.count(id) != 0;
        }
        const auto textures() const
        {
            return mTextures;
        }
        size_t highestID() const
        {
            return mHighestID;
        }
        void reloadCache();

      private:
        uintptr_t ptr{0};
        std::unordered_map<uint32_t, std::pair<std::string, uintptr_t>> mTextures; // id -> {name, address}
        QStringList mTextureNamesStringList;
        size_t mHighestID;

        TextureDB() = default;
        ~TextureDB(){};
        TextureDB(const TextureDB&) = delete;
        TextureDB& operator=(const TextureDB&) = delete;

        friend struct Spelunky2;
    };
} // namespace S2Plugin
