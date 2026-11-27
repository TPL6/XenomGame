#pragma once

#include <memory>

enum class GameAction {
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    Fire
};

class InputManager {
public:
    InputManager();
    ~InputManager();

    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    // Polls OS/window/input events and updates keyboard + gamepad state.
    void pollEvents();
    void shutdown();

    bool isActionDown(GameAction action) const;
    bool quitRequested() const noexcept;
    bool isPaused() const noexcept;
    void setPaused(bool paused) noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
