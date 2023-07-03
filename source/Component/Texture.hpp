#pragma once

// OPENGL
#include "Types.hpp"

// UTILITY
#include "ResourceManager.hpp"
#include "File.hpp"

// STD
#include <filesystem>
#include <optional>
#include <string>

namespace System
{
    class TextureSystem;
}

namespace Data
{
    // Texture represents an image file on disk and its associated GPU handle.
    // On construction a Texture is loaded into memory and onto the GPU ready for rendering.
    class Texture
    {
    public:
        Texture(const std::filesystem::path& p_filePath);
        ~Texture() = default;

        Utility::File::ImageRef m_image_ref;
        OpenGL::Texture m_GL_texture;
    };
}

using TextureManager = Utility::ResourceManager<Data::Texture>;
using TextureRef     = Utility::ResourceRef<Data::Texture>;

namespace Component
{
    class Texture
    {
    public:
        Texture() noexcept;
        Texture(const TextureRef& m_diffuse) noexcept;

        std::optional<TextureRef> mDiffuse;
        std::optional<TextureRef> mSpecular;
        float m_shininess;
    };
}; // namespace Component