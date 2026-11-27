#pragma once

#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

using TextureHandle = std::uint64_t;
constexpr TextureHandle kInvalidTextureHandle = 0;

struct UVRect {
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 1.0f;
    float v1 = 1.0f;

    static UVRect fromPixelRect(
        float x, float y, float w, float h,
        float texWidth, float texHeight
    ) noexcept {
        if (texWidth <= 0.0f || texHeight <= 0.0f) {
            return {};
        }
        return UVRect{
            x / texWidth,
            y / texHeight,
            (x + w) / texWidth,
            (y + h) / texHeight
        };
    }
};

struct SpriteDrawDesc {
    TextureHandle texture = kInvalidTextureHandle;
    UVRect uv{};
    glm::vec2 position{ 0.0f, 0.0f }; // World/screen pixel position
    glm::vec2 size{ 32.0f, 32.0f };    // Base quad size in pixels
    glm::vec2 scale{ 1.0f, 1.0f };
    glm::vec2 origin{ 0.5f, 0.5f };    // Normalized pivot (0.5, 0.5 = center)
    float rotationRadians = 0.0f;
    glm::vec4 tint{ 1.0f, 1.0f, 1.0f, 1.0f };
};

struct ParallaxLayerDesc {
    TextureHandle texture = kInvalidTextureHandle;
    UVRect uv{};
    glm::vec2 scrollOffset{ 0.0f, 0.0f }; // UV scroll offset
    glm::vec2 uvScale{ 1.0f, 1.0f };
    glm::vec2 position{ 0.0f, 0.0f };
    glm::vec2 size{ 640.0f, 480.0f };
    glm::vec4 tint{ 1.0f, 1.0f, 1.0f, 1.0f };
};

class Renderer {
public:
    virtual ~Renderer() = default;

    virtual void beginFrame(int viewportWidth, int viewportHeight) = 0;
    virtual void endFrame() = 0;

    virtual void drawSprite(const SpriteDrawDesc& desc) = 0;
    virtual void drawParallaxLayer(const ParallaxLayerDesc& desc) = 0;
};
