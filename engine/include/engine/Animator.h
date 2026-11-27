#pragma once

#include "engine/Renderer.h"

#include <memory>
#include <vector>

class Animator {
public:
    void reset(float frameDuration, bool loop);
    void setGrid(int columns, int rows, float frameDuration, bool loop, int maxFrames = 0);
    void setFrames(std::vector<UVRect> frames, float frameDuration, bool loop);
    void setFrame(int index);
    void update(float dt);

    int currentFrame() const noexcept { return currentFrame_; }
    int frameCount() const noexcept { return frames_ ? static_cast<int>(frames_->size()) : 0; }
    bool isFinished() const noexcept { return finished_; }
    UVRect currentUV() const;

private:
    std::shared_ptr<const std::vector<UVRect>> frames_;
    int currentFrame_ = 0;
    float frameTimer_ = 0.0f;
    float frameDuration_ = 0.1f;
    bool loop_ = true;
    bool finished_ = false;
};
