#include "engine/Actor.h"

void Actor::setSprite(
    TextureHandle texture,
    glm::vec2 quadSize,
    int columns,
    int rows,
    float frameDuration,
    bool loop,
    int maxFrames)
{
    texture_ = texture;
    size_ = quadSize;
    animator_.setGrid(columns, rows, frameDuration, loop, maxFrames);
}

void Actor::setUVFrames(std::vector<UVRect> frames, float frameDuration, bool loop) {
    animator_.setFrames(std::move(frames), frameDuration, loop);
}

void Actor::setCurrentFrame(int index) {
    animator_.setFrame(index);
}

void Actor::updateAnimation(float dt) {
    animator_.update(dt);
}

void Actor::syncFromPhysics(float pixelsPerMeter) {
    if (!body_.isValid() || pixelsPerMeter <= 0.0f) {
        return;
    }

    position_ = body_.positionPixels(pixelsPerMeter);
    if (!body_.isFixedRotation()) {
        rotationRadians_ = body_.rotationRadians();
    }
}

void Actor::syncToPhysics(float pixelsPerMeter) {
    if (!body_.isValid() || pixelsPerMeter <= 0.0f) {
        return;
    }

    body_.setTransformPixels(position_, rotationRadians_, pixelsPerMeter);
}

void Actor::syncTargetToPhysics(float dt, float pixelsPerMeter) {
    if (!body_.isValid() || pixelsPerMeter <= 0.0f || dt <= 0.0f) {
        return;
    }

    body_.setTargetTransformPixels(position_, rotationRadians_, dt, pixelsPerMeter);
}

void Actor::update(float dt, const InputManager& /*input*/, ResourceManager& /*resources*/) {
    animator_.update(dt);
}

void Actor::render(Renderer& renderer) const {
    if (pendingDestroy_ || texture_ == kInvalidTextureHandle) {
        return;
    }

    renderer.drawSprite(SpriteDrawDesc{
        .texture = texture_,
        .uv = animator_.currentUV(),
        .position = position_,
        .size = size_,
        .scale = scale_,
        .origin = origin_,
        .rotationRadians = rotationRadians_,
        .tint = tint_,
    });
}
