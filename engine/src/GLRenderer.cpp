#include "GLRenderer.h"
#include "Window.h"
#include "engine/ResourceManager.h"

#include <array>
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace {

    constexpr const char* kFallbackVertexShaderSrc = R"glsl(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 aTexCoord;

uniform mat4 uProjection;
uniform mat4 uModel;
uniform vec4 uUVRect;
uniform vec2 uUVScroll;
uniform vec2 uUVScale;

out vec2 vTexCoord;

void main() {
    vec2 subUV = mix(uUVRect.xy, uUVRect.zw, aTexCoord);
    vTexCoord = subUV * uUVScale + uUVScroll;
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
}
)glsl";

    constexpr const char* kFallbackFragmentShaderSrc = R"glsl(
#version 330 core
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform vec4 uTint;

void main() {
    vec4 texColor = texture(uTexture, vTexCoord);
    if (texColor.a <= 0.001 ||
        (texColor.r > 0.99 && texColor.g < 0.01 && texColor.b > 0.99)) {
        discard;
    }
    FragColor = texColor * uTint;
}
)glsl";

} // namespace

GLRenderer::GLRenderer(
    const Window& window,
    ResourceManager& resources,
    int initialWidth,
    int initialHeight)
    : window_(&window)
    , viewportWidth_(initialWidth > 0 ? initialWidth : 640)
    , viewportHeight_(initialHeight > 0 ? initialHeight : 480)
{
    if (!window.isValid()) {
        std::cerr << "GLRenderer: window is invalid\n";
        window_ = nullptr;
        return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);

    initQuadGeometry();
    initShaderProgram(resources);

    projection_ = glm::ortho(
        0.0f, static_cast<float>(viewportWidth_),
        static_cast<float>(viewportHeight_), 0.0f,
        -1.0f, 1.0f
    );
}

void GLRenderer::initQuadGeometry() {
    // 4 unique vertices for a unit quad in [0, 1] x [0, 1] (top-left origin matching glm::ortho)
    constexpr std::array<float, 16> vertices = {
        // pos (x, y)   // texCoord (u, v)
        0.0f, 0.0f,     0.0f, 0.0f, // 0: top-left
        1.0f, 0.0f,     1.0f, 0.0f, // 1: top-right
        1.0f, 1.0f,     1.0f, 1.0f, // 2: bottom-right
        0.0f, 1.0f,     0.0f, 1.0f  // 3: bottom-left
    };

    constexpr std::array<unsigned int, 6> indices = {
        0, 1, 2,
        0, 2, 3
    };

    vao_ = VertexArray::create();
    vao_.bind();

    vbo_ = VertexBuffer::create(vertices.data(), vertices.size() * sizeof(float));
    ebo_ = IndexBuffer::create(indices.data(), indices.size());

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), nullptr
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), reinterpret_cast<const void*>(2 * sizeof(float))
    );

    // Note: Do NOT unbind EBO before unbinding VAO, as the VAO records the EBO binding.
    VertexArray::unbind();
    VertexBuffer::unbind();
}

void GLRenderer::initShaderProgram(ResourceManager& resources) {
    shader_ = resources.loadShader("sprite", "shaders/sprite.vert", "shaders/sprite.frag");
    if (shader_ == nullptr || !shader_->isValid()) {
        std::cerr << "GLRenderer: falling back to embedded sprite shader\n";
        fallbackShader_ = Shader::fromSource(
            kFallbackVertexShaderSrc,
            kFallbackFragmentShaderSrc,
            "fallback_sprite"
        );
        shader_ = fallbackShader_.isValid() ? &fallbackShader_ : nullptr;
    }

    if (shader_ != nullptr && shader_->isValid()) {
        shader_->bind();
        shader_->setInt("uTexture", 0);
        Shader::unbind();
    }
}

void GLRenderer::beginFrame(int viewportWidth, int viewportHeight) {
    if (!isValid()) {
        return;
    }

    viewportWidth_ = (viewportWidth > 0) ? viewportWidth : 1;
    viewportHeight_ = (viewportHeight > 0) ? viewportHeight : 1;

    glViewport(0, 0, viewportWidth_, viewportHeight_);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    projection_ = glm::ortho(
        0.0f, static_cast<float>(viewportWidth_),
        static_cast<float>(viewportHeight_), 0.0f,
        -1.0f, 1.0f
    );
}

void GLRenderer::endFrame() {
    if (window_ != nullptr) {
        window_->swapBuffers();
    }
}

void GLRenderer::drawSprite(const SpriteDrawDesc& desc) {
    if (desc.texture == kInvalidTextureHandle || !isValid()) {
        return;
    }

    const glm::vec2 scaledSize = desc.size * desc.scale;
    const glm::vec2 pivotOffset = -desc.origin * scaledSize;

    // Matrix order (applied right-to-left): Scale -> Pivot Offset -> Rotate -> Translate
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(desc.position, 0.0f));
    if (desc.rotationRadians != 0.0f) {
        model = glm::rotate(model, desc.rotationRadians, glm::vec3(0.0f, 0.0f, 1.0f));
    }
    model = glm::translate(model, glm::vec3(pivotOffset, 0.0f));
    model = glm::scale(model, glm::vec3(scaledSize, 1.0f));

    drawQuad(
        static_cast<unsigned int>(desc.texture),
        model,
        desc.uv,
        glm::vec2{ 0.0f, 0.0f },
        glm::vec2{ 1.0f, 1.0f },
        desc.tint
    );
}

void GLRenderer::drawParallaxLayer(const ParallaxLayerDesc& desc) {
    if (desc.texture == kInvalidTextureHandle || !isValid()) {
        return;
    }

    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(desc.position, 0.0f));
    model = glm::scale(model, glm::vec3(desc.size, 1.0f));

    drawQuad(
        static_cast<unsigned int>(desc.texture),
        model,
        desc.uv,
        desc.scrollOffset,
        desc.uvScale,
        desc.tint
    );
}

void GLRenderer::drawQuad(
    unsigned int glTextureId,
    const glm::mat4& model,
    const UVRect& uv,
    const glm::vec2& uvScroll,
    const glm::vec2& uvScale,
    const glm::vec4& tint)
{
    shader_->bind();
    shader_->setMat4("uProjection", projection_);
    shader_->setMat4("uModel", model);
    shader_->setVec4("uUVRect", glm::vec4(uv.u0, uv.v0, uv.u1, uv.v1));
    shader_->setVec2("uUVScroll", uvScroll);
    shader_->setVec2("uUVScale", uvScale);
    shader_->setVec4("uTint", tint);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, glTextureId);

    vao_.bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(ebo_.count()), GL_UNSIGNED_INT, nullptr);
    VertexArray::unbind();

    Shader::unbind();
}
