#pragma once

#include "Texture.hpp"

#include <array>
#include <unordered_map>
#include <functional>

namespace std
{
    namespace filesystem
    {
        class path;
        class directory_entry;
    }
}

class TextureManager
{
public:
    TextureManager();
    TextureID getTextureID(const std::string& pTextureName) const;
    std::string getTextureName(const TextureID& pTextureID) const;

    inline void ForEach(const std::function<void(const Texture&)>& pFunction) const
    {
        for (const auto& Texture : mTextures)
            pFunction(Texture);
    }

    inline void ForEachCubeMap(const std::function<void(const CubeMapTexture&)>& pFunction) const
    {
        for (size_t i = 0; i < mCubeMaps.size(); i++)
            pFunction(mCubeMaps[i]);
    }

    // Loads individual texture data at pFilePath using stb_image.
    // The Texture is added to the TextureManager store and the pointer to it returned.
    Texture& loadTexture(const std::filesystem::path& pFilePath, const Texture::Purpose pPurpose = Texture::Purpose::Diffuse, const std::string& pName = "");
    // Loads all the cubemap textures. pCubeMapDirectory is the root of all the cubemaps individually storing 6 textures per folder.
    void loadCubeMaps(const std::filesystem::directory_entry& pCubeMapDirectory);
private:
    std::vector<Texture> mTextures;
    std::unordered_map<std::string, size_t> mNameLookup;
    std::unordered_map<std::string, size_t> mFilePathLookup;

    std::vector<CubeMapTexture> mCubeMaps;
};