#include "engine/Texture.h"

#include "AssetPath.h"

#include <SDL3/SDL.h>
#include <cstddef>
#include <cstdint>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {

    struct SurfaceDeleter {
        void operator()(SDL_Surface* surface) const noexcept {
            if (surface != nullptr) {
                SDL_DestroySurface(surface);
            }
        }
    };
    using SurfacePtr = std::unique_ptr<SDL_Surface, SurfaceDeleter>;

    GLenum toGLWrap(TextureWrapMode mode) noexcept {
        switch (mode) {
        case TextureWrapMode::Repeat:
            return GL_REPEAT;
        case TextureWrapMode::MirroredRepeat:
            return GL_MIRRORED_REPEAT;
        case TextureWrapMode::ClampToEdge:
        default:
            return GL_CLAMP_TO_EDGE;
        }
    }

    GLenum toGLFilter(TextureFilterMode mode) noexcept {
        switch (mode) {
        case TextureFilterMode::Linear:
            return GL_LINEAR;
        case TextureFilterMode::Nearest:
        default:
            return GL_NEAREST;
        }
    }

} // namespace

Texture::Texture(
    std::string_view bmpPath,
    bool magentaTransparent,
    TextureWrapMode wrapMode,
    TextureFilterMode filterMode)
{
    const std::string resolvedPath = resolveAssetPath(bmpPath);
    SurfacePtr rawSurface(SDL_LoadBMP(resolvedPath.c_str()));
    if (!rawSurface) {
        std::cerr << "Texture: failed to load BMP '" << bmpPath << "': " << SDL_GetError() << "\n";
        return;
    }

    SurfacePtr rgbaSurface(SDL_ConvertSurface(rawSurface.get(), SDL_PIXELFORMAT_RGBA32));
    rawSurface.reset();

    if (!rgbaSurface) {
        std::cerr << "Texture: failed to convert '" << bmpPath << "' to RGBA32: " << SDL_GetError() << "\n";
        return;
    }

    width_ = rgbaSurface->w;
    height_ = rgbaSurface->h;

    // Copy pixels into a tightly packed row-major buffer (top-to-bottom matching top-left UV origin).
    std::vector<std::uint8_t> pixels(
        static_cast<std::size_t>(width_) * static_cast<std::size_t>(height_) * 4u
    );
    const auto* srcRows = static_cast<const std::uint8_t*>(rgbaSurface->pixels);
    const std::size_t rowBytes = static_cast<std::size_t>(width_) * 4u;

    for (int y = 0; y < height_; ++y) {
        const std::uint8_t* srcRow = srcRows + static_cast<std::size_t>(y) * static_cast<std::size_t>(rgbaSurface->pitch);
        std::uint8_t* dstRow = pixels.data() + static_cast<std::size_t>(y) * rowBytes;

        for (int x = 0; x < width_; ++x) {
            const std::size_t idx = static_cast<std::size_t>(x) * 4u;
            const std::uint8_t r = srcRow[idx + 0];
            const std::uint8_t g = srcRow[idx + 1];
            const std::uint8_t b = srcRow[idx + 2];
            std::uint8_t a = srcRow[idx + 3];

            if (magentaTransparent && r == 255 && g == 0 && b == 255) {
                a = 0;
            }

            dstRow[idx + 0] = r;
            dstRow[idx + 1] = g;
            dstRow[idx + 2] = b;
            dstRow[idx + 3] = a;
        }
    }

    rgbaSurface.reset();

    unsigned int glTex = 0;
    glGenTextures(1, &glTex);
    glBindTexture(GL_TEXTURE_2D, glTex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        width_,
        height_,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );

    textureId_ = glTex;
    setWrapMode(wrapMode);
    setFilterMode(filterMode);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    release();
}

Texture::Texture(Texture&& other) noexcept
    : textureId_(other.textureId_)
    , width_(other.width_)
    , height_(other.height_) {
    other.textureId_ = 0;
    other.width_ = 0;
    other.height_ = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept {
    if (this != &other) {
        release();
        textureId_ = other.textureId_;
        width_ = other.width_;
        height_ = other.height_;
        other.textureId_ = 0;
        other.width_ = 0;
        other.height_ = 0;
    }
    return *this;
}

void Texture::release() {
    if (textureId_ != 0) {
        glDeleteTextures(1, &textureId_);
        textureId_ = 0;
    }
    width_ = 0;
    height_ = 0;
}

void Texture::bind(unsigned int unit) const {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, textureId_);
}

void Texture::unbind(unsigned int unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setWrapMode(TextureWrapMode wrapMode) {
    if (textureId_ == 0) {
        return;
    }
    const GLenum glWrap = toGLWrap(wrapMode);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::setFilterMode(TextureFilterMode filterMode) {
    if (textureId_ == 0) {
        return;
    }
    const GLenum glFilter = toGLFilter(filterMode);
    glBindTexture(GL_TEXTURE_2D, textureId_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter);
    glBindTexture(GL_TEXTURE_2D, 0);
}
