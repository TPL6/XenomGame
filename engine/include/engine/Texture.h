#pragma once

#include <string_view>

enum class TextureWrapMode {
    ClampToEdge,
    Repeat,
    MirroredRepeat
};

enum class TextureFilterMode {
    Nearest,
    Linear
};

class Texture {
public:
    Texture() = default;
    Texture(
        std::string_view bmpPath,
        bool magentaTransparent = true,
        TextureWrapMode wrapMode = TextureWrapMode::ClampToEdge,
        TextureFilterMode filterMode = TextureFilterMode::Nearest
    );
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    Texture(Texture&& other) noexcept;
    Texture& operator=(Texture&& other) noexcept;

    bool isValid() const noexcept { return textureId_ != 0; }
    unsigned int id() const noexcept { return textureId_; }
    int width() const noexcept { return width_; }
    int height() const noexcept { return height_; }

    void bind(unsigned int unit = 0) const;
    static void unbind(unsigned int unit = 0);

    void setWrapMode(TextureWrapMode wrapMode);
    void setFilterMode(TextureFilterMode filterMode);

private:
    void release();

    unsigned int textureId_ = 0;
    int width_ = 0;
    int height_ = 0;
};
