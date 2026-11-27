#pragma once

#include "GLBuffers.h"
#include "engine/Renderer.h"
#include "engine/Shader.h"

#include <glm/mat4x4.hpp>

class ResourceManager;
class Window;

class GLRenderer final : public Renderer {
public:
    GLRenderer(
        const Window& window,
        ResourceManager& resources,
        int initialWidth,
        int initialHeight
    );
    ~GLRenderer() override = default;

    GLRenderer(const GLRenderer&) = delete;
    GLRenderer& operator=(const GLRenderer&) = delete;

    bool isValid() const noexcept {
        return window_ != nullptr && vao_.isValid() && vbo_.isValid() && ebo_.isValid() && shader_ != nullptr && shader_->isValid();
    }

    void beginFrame(int viewportWidth, int viewportHeight) override;
    void endFrame() override;

    void drawSprite(const SpriteDrawDesc& desc) override;
    void drawParallaxLayer(const ParallaxLayerDesc& desc) override;

private:
    void initQuadGeometry();
    void initShaderProgram(ResourceManager& resources);

    void drawQuad(
        unsigned int glTextureId,
        const glm::mat4& model,
        const UVRect& uv,
        const glm::vec2& uvScroll,
        const glm::vec2& uvScale,
        const glm::vec4& tint
    );

    const Window* window_ = nullptr;

    VertexArray vao_;
    VertexBuffer vbo_;
    IndexBuffer ebo_;

    const Shader* shader_ = nullptr;
    Shader fallbackShader_;

    glm::mat4 projection_{ 1.0f };
    int viewportWidth_ = 640;
    int viewportHeight_ = 480;
};
