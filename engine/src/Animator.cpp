#include "engine/Animator.h"
#include "engine/SpriteSheet.h"

#include <algorithm>

namespace {
    constexpr UVRect kFullUV{ 0.0f, 0.0f, 1.0f, 1.0f };
} // namespace

void Animator::reset(float frameDuration, bool loop) {
    currentFrame_ = 0;
    frameTimer_ = 0.0f;
    frameDuration_ = frameDuration;
    loop_ = loop;
    finished_ = false;
}

void Animator::setGrid(int columns, int rows, float frameDuration, bool loop, int maxFrames) {
    frames_ = getSpriteSheetUVs(std::max(1, columns), std::max(1, rows), maxFrames);
    reset(frameDuration, loop);
}

void Animator::setFrames(std::vector<UVRect> frames, float frameDuration, bool loop) {
    frames_ = std::make_shared<const std::vector<UVRect>>(std::move(frames));
    reset(frameDuration, loop);
}

void Animator::setFrame(int index) {
    if (!frames_ || frames_->empty()) {
        currentFrame_ = 0;
        return;
    }
    currentFrame_ = std::clamp(index, 0, static_cast<int>(frames_->size()) - 1);
}

void Animator::update(float dt) {
    if (!frames_ || frames_->empty()) {
        currentFrame_ = 0;
        finished_ = true;
        return;
    }

    const int frameCount = static_cast<int>(frames_->size());
    const int maxFrame = frameCount - 1;
    currentFrame_ = std::clamp(currentFrame_, 0, maxFrame);

    if (!loop_ && currentFrame_ == maxFrame) {
        finished_ = true;
        return;
    }

    if (frameDuration_ <= 0.0f || dt <= 0.0f) {
        return;
    }

    frameTimer_ += dt;
    while (frameTimer_ >= frameDuration_) {
        frameTimer_ -= frameDuration_;

        if (currentFrame_ < maxFrame) {
            ++currentFrame_;
            if (!loop_ && currentFrame_ == maxFrame) {
                finished_ = true;
            }
        }
        else if (loop_) {
            currentFrame_ = 0;
            finished_ = false;
        }
        else {
            currentFrame_ = maxFrame;
            frameTimer_ = 0.0f;
            finished_ = true;
            break;
        }
    }
}

UVRect Animator::currentUV() const {
    if (!frames_ || frames_->empty()) {
        return kFullUV;
    }
    return (*frames_)[static_cast<std::size_t>(currentFrame_)];
}
