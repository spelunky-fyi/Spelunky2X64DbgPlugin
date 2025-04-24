#include "Data/TextureDB.h"
#include "pluginmain.h"
#include "read_helpers.h"

const std::string& S2Plugin::TextureDB::nameForID(uint32_t id) const
{
    if (auto it = mTextures.find(id); it != mTextures.end())
    {
        return it->second.first;
    }
    static std::string unknownName("UNKNOWN TEXTURE");
    return unknownName;
}

void S2Plugin::TextureDB::reloadCache()
{
    if (!isValid())
        return;

    mTextures.clear();
    mTextureNamesStringList.clear();
    mHighestID = 0;

    auto textureCount = Script::Memory::ReadQword(ptr - 0x8);
    constexpr uintptr_t textureSize = 0x40ull;
    for (size_t x = 0; x < (std::min)(500ull, textureCount); ++x)
    {
        uintptr_t offset = ptr + textureSize * x;
        auto textureID = Script::Memory::ReadQword(offset);
        mHighestID = std::max(mHighestID, textureID);

        auto nameOffset = offset + 0x8;

        size_t value = (nameOffset == 0 ? 0 : Script::Memory::ReadQword(Script::Memory::ReadQword(nameOffset)));
        if (value != 0)
        {
            std::string name = ReadConstString(value);

            mTextureNamesStringList << QString("Texture %1 (%2)").arg(textureID).arg(QString::fromStdString(name));
            mTextures.emplace(textureID, std::make_pair(std::move(name), offset));
        }
    }
}
