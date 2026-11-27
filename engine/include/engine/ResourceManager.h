#pragma once

#include "engine/Renderer.h"
#include "engine/Shader.h"
#include "engine/Texture.h"

#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

class ResourceManager {
public:
    ResourceManager() = default;
    ~ResourceManager() { clear(); }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    const Texture* loadTexture(
        const std::string& key,
        std::string_view filePath,
        bool magentaTransparent = true,
        TextureWrapMode wrapMode = TextureWrapMode::ClampToEdge,
        TextureFilterMode filterMode = TextureFilterMode::Nearest
    );
    const Texture* getTexture(const std::string& key) const;
    TextureHandle getTextureHandle(const std::string& key) const;

    const Shader* loadShader(
        const std::string& key,
        std::string_view vertexPath,
        std::string_view fragmentPath
    );
    const Shader* getShader(const std::string& key) const;

    void clear();

private:
    std::unordered_map<std::string, std::shared_ptr<Texture>> texturesByKey_;
    std::unordered_map<std::string, std::string> texturePathToKey_;
    std::unordered_map<std::string, std::shared_ptr<Shader>> shadersByKey_;
    std::unordered_map<std::string, std::string> shaderPathPairToKey_;
};
