#include "engine/ResourceManager.h"

#include <iostream>

const Texture* ResourceManager::loadTexture(
    const std::string& key,
    std::string_view filePath,
    bool magentaTransparent,
    TextureWrapMode wrapMode,
    TextureFilterMode filterMode)
{
    if (const auto keyIt = texturesByKey_.find(key); keyIt != texturesByKey_.end()) {
        return keyIt->second.get();
    }

    const std::string pathStr(filePath);
    if (const auto pathIt = texturePathToKey_.find(pathStr); pathIt != texturePathToKey_.end()) {
        if (const auto cachedIt = texturesByKey_.find(pathIt->second); cachedIt != texturesByKey_.end()) {
            texturesByKey_[key] = cachedIt->second;
            return cachedIt->second.get();
        }
    }

    auto texture = std::make_shared<Texture>(filePath, magentaTransparent, wrapMode, filterMode);
    if (!texture->isValid()) {
        std::cerr << "ResourceManager: failed to load texture '" << filePath << "'\n";
        return nullptr;
    }

    texturesByKey_[key] = texture;
    texturePathToKey_[pathStr] = key;
    return texture.get();
}

const Texture* ResourceManager::getTexture(const std::string& key) const {
    if (const auto it = texturesByKey_.find(key); it != texturesByKey_.end()) {
        return it->second.get();
    }
    return nullptr;
}

TextureHandle ResourceManager::getTextureHandle(const std::string& key) const {
    const Texture* tex = getTexture(key);
    return (tex != nullptr && tex->isValid())
        ? static_cast<TextureHandle>(tex->id())
        : kInvalidTextureHandle;
}

const Shader* ResourceManager::loadShader(
    const std::string& key,
    std::string_view vertexPath,
    std::string_view fragmentPath)
{
    if (const auto keyIt = shadersByKey_.find(key); keyIt != shadersByKey_.end()) {
        return keyIt->second.get();
    }

    const std::string combinedPath = std::string(vertexPath) + "|" + std::string(fragmentPath);
    if (const auto pathIt = shaderPathPairToKey_.find(combinedPath); pathIt != shaderPathPairToKey_.end()) {
        if (const auto cachedIt = shadersByKey_.find(pathIt->second); cachedIt != shadersByKey_.end()) {
            shadersByKey_[key] = cachedIt->second;
            return cachedIt->second.get();
        }
    }

    auto shader = std::make_shared<Shader>(vertexPath, fragmentPath);
    if (!shader->isValid()) {
        std::cerr << "ResourceManager: failed to load shader (" << vertexPath << ", " << fragmentPath << ")\n";
        return nullptr;
    }

    shadersByKey_[key] = shader;
    shaderPathPairToKey_[combinedPath] = key;
    return shader.get();
}

const Shader* ResourceManager::getShader(const std::string& key) const {
    if (const auto it = shadersByKey_.find(key); it != shadersByKey_.end()) {
        return it->second.get();
    }
    return nullptr;
}

void ResourceManager::clear() {
    shadersByKey_.clear();
    shaderPathPairToKey_.clear();
    texturesByKey_.clear();
    texturePathToKey_.clear();
}
