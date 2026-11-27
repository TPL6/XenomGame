#include "Explosion.h"

#include "engine/InputManager.h"
#include "engine/Level.h"
#include "engine/Renderer.h"
#include "engine/ResourceManager.h"

namespace {
    constexpr int kExplosionColumns = 5;
    constexpr int kExplosionRows = 2;
} // namespace

Explosion::Explosion(glm::vec2 pos, glm::vec2 quadSize, float frameDuration)
    : frameDuration_(frameDuration)
{
    setPosition(pos);
    setSize(quadSize);
    setOrigin({ 0.5f, 0.5f });
    setDrawOrder(30);
}

void Explosion::onSpawn(Level& /*level*/, ResourceManager& resources) {
    const Texture* tex = resources.loadTexture(
        "explosion",
        "graphics/explode64.bmp",
        true,
        TextureWrapMode::ClampToEdge,
        TextureFilterMode::Nearest
    );

    if (tex == nullptr || !tex->isValid()) {
        setSprite(kInvalidTextureHandle, size(), 1, 1, frameDuration_, false);
        destroy();
        return;
    }

    const auto handle = static_cast<TextureHandle>(tex->id());
    setSprite(handle, size(), kExplosionColumns, kExplosionRows, frameDuration_, false);
}

void Explosion::update(float dt, const InputManager& /*input*/, ResourceManager& /*resources*/) {
    if (isPendingDestroy()) {
        return;
    }

    setRotation(rotation() + spinSpeed_ * dt);
    updateAnimation(dt);

    if (isAnimationFinished()) {
        if (level() != nullptr) {
            level()->queueDestroyActor(this);
        }
        else {
            destroy();
        }
    }
}
