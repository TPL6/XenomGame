#include "VerticalXenonLevel.h"

#include "LonerEnemy.h"
#include "PlayerShip.h"
#include "RusherEnemy.h"

#include "engine/InputManager.h"
#include "engine/ResourceManager.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace {

    constexpr float kInitialWaveDelay = 0.75f;
    constexpr float kLonerWaveInterval = 3.2f;
    constexpr float kRusherWaveInterval = 2.2f;
    constexpr float kPlayerRespawnDelay = 1.5f;

    float wrapPeriod(float value, float period) {
        const float mod = std::fmod(value, period);
        return (mod < 0.0f) ? (mod + period) : mod;
    }

    TextureHandle toTextureHandle(const Texture* tex) noexcept {
        return (tex && tex->isValid())
            ? static_cast<TextureHandle>(tex->id())
            : kInvalidTextureHandle;
    }

} // namespace

VerticalXenonLevel::VerticalXenonLevel(float width, float height)
    : Level({ 0.0f, 0.0f }, 1.0f / 60.0f, 4, 32.0f)
    , width_(width > 0.0f ? width : 640.0f)
    , height_(height > 0.0f ? height : 480.0f)
    , rng_(std::random_device{}())
{
}

void VerticalXenonLevel::setupParallaxLayers(ResourceManager& resources) {
    clearParallaxLayers();
    farLayerIdx_ = -1;

    const Texture* farTex = resources.loadTexture(
        "bg_far",
        "graphics/galaxy2.bmp",
        false,
        TextureWrapMode::MirroredRepeat,
        TextureFilterMode::Linear
    );
    farBgHandle_ = toTextureHandle(farTex);

    if (farBgHandle_ != kInvalidTextureHandle) {
        ParallaxLayerDesc farLayer{};
        farLayer.texture = farBgHandle_;
        farLayer.uv = UVRect{ 0.0f, 0.0f, 1.0f, 1.0f };
        farLayer.scrollOffset = farScroll_;
        farLayer.uvScale = { 1.0f, 1.0f };
        farLayer.position = { 0.0f, 0.0f };
        farLayer.size = { width_, height_ };
        farLayer.tint = { 1.0f, 1.0f, 1.0f, 1.0f };
        farLayerIdx_ = static_cast<int>(parallaxLayers().size());
        addParallaxLayer(farLayer);
    }
}

void VerticalXenonLevel::onEnter(ResourceManager& resources) {
    farScroll_ = { 0.0f, 0.0f };

    setupParallaxLayers(resources);

    PlayerShip* ship = spawnActor<PlayerShip>(
        glm::vec2{ width_ * 0.5f, height_ - 64.0f },
        width_,
        height_
    );

    flushSpawns(resources);
    playerId_ = (ship != nullptr) ? ship->id() : 0;
    respawnTimer_ = 0.0f;

    waveTimer_ = kInitialWaveDelay;
    waveIndex_ = 0;
}

void VerticalXenonLevel::onExit(ResourceManager& resources) {
    playerId_ = 0;
    Level::onExit(resources);
}

void VerticalXenonLevel::updateParallaxScroll(float dt) {
    if (dt <= 0.0f) {
        return;
    }

    farScroll_.y = wrapPeriod(farScroll_.y - farScrollSpeed_ * dt, 2.0f);

    auto& layers = parallaxLayers();
    if (farLayerIdx_ >= 0 && static_cast<std::size_t>(farLayerIdx_) < layers.size()) {
        layers[static_cast<std::size_t>(farLayerIdx_)].scrollOffset = farScroll_;
    }
}

void VerticalXenonLevel::updatePlayerRespawn(float dt) {
    if (playerId_ != 0 && findActorById(playerId_) != nullptr) {
        respawnTimer_ = 0.0f;
        return;
    }

    playerId_ = 0;
    respawnTimer_ += dt;
    if (respawnTimer_ >= kPlayerRespawnDelay) {
        respawnTimer_ = 0.0f;
        spawnActor<PlayerShip>(
            glm::vec2{ width_ * 0.5f, height_ - 64.0f },
            width_,
            height_
        );
    }
}

void VerticalXenonLevel::spawnLonerWave() {
    std::uniform_real_distribution<float> yDist(64.0f, std::max(96.0f, height_ * 0.45f));
    const float baseDirX = ((waveIndex_ / 2) % 2 == 0) ? 1.0f : -1.0f;

    constexpr int kLonerCount = 3;
    for (int i = 0; i < kLonerCount; ++i) {
        const float startX = (baseDirX > 0.0f)
            ? (-32.0f - static_cast<float>(i) * 90.0f)
            : (width_ + 32.0f + static_cast<float>(i) * 90.0f);
        const glm::vec2 pos{ startX, yDist(rng_) };
        spawnActor<LonerEnemy>(pos, baseDirX, width_, height_);
    }
}

void VerticalXenonLevel::spawnRusherWave() {
    const float minX = std::min(64.0f, width_ * 0.5f);
    const float maxX = std::max(minX, width_ - 64.0f);
    std::uniform_real_distribution<float> xDist(minX, maxX);

    constexpr int kRusherCount = 2;
    for (int i = 0; i < kRusherCount; ++i) {
        const glm::vec2 pos{ xDist(rng_), -32.0f - static_cast<float>(i) * 56.0f };
        spawnActor<RusherEnemy>(pos, 250.0f, height_);
    }
}

void VerticalXenonLevel::updateWaves(float dt) {
    if (dt <= 0.0f) {
        return;
    }

    waveTimer_ -= dt;
    if (waveTimer_ > 0.0f) {
        return;
    }

    if (waveIndex_ % 2 == 0) {
        spawnLonerWave();
        waveTimer_ = kLonerWaveInterval;
    }
    else {
        spawnRusherWave();
        waveTimer_ = kRusherWaveInterval;
    }
    ++waveIndex_;
}

void VerticalXenonLevel::fixedUpdate(float dt, const InputManager& input, ResourceManager& resources) {
    updateParallaxScroll(dt);
    updatePlayerRespawn(dt);
    updateWaves(dt);

    Level::fixedUpdate(dt, input, resources);

    if (playerId_ == 0) {
        for (const auto& actor : actors()) {
            if (auto* ship = dynamic_cast<PlayerShip*>(actor.get());
                ship && !ship->isPendingDestroy()) {
                playerId_ = ship->id();
                break;
            }
        }
    }
}

void VerticalXenonLevel::render(Renderer& renderer, ResourceManager& resources, float alpha) {
    Level::render(renderer, resources, alpha);
}
