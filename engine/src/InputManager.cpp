#include "engine/InputManager.h"

#include <SDL3/SDL.h>
#include <algorithm>
#include <memory>
#include <vector>

namespace {

    struct GamepadDeleter {
        void operator()(SDL_Gamepad* pad) const noexcept {
            if (pad != nullptr) {
                SDL_CloseGamepad(pad);
            }
        }
    };
    using GamepadPtr = std::unique_ptr<SDL_Gamepad, GamepadDeleter>;

} // namespace

struct InputManager::Impl {
    std::vector<GamepadPtr> pads;
    const bool* keyState = nullptr;
    bool shouldQuit = false;
    bool paused = false;

    void syncGamepads() {
        if (!pads.empty()) {
            return;
        }

        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (ids != nullptr) {
            for (int i = 0; i < count; ++i) {
                if (SDL_Gamepad* pad = SDL_OpenGamepad(ids[i])) {
                    pads.emplace_back(pad);
                }
            }
            SDL_free(ids);
        }
    }

    void processEvent(const SDL_Event& ev) {
        if (ev.type == SDL_EVENT_QUIT) {
            shouldQuit = true;
        }
        else if (ev.type == SDL_EVENT_KEY_DOWN && !ev.key.repeat) {
            if (ev.key.key == SDLK_ESCAPE) {
                shouldQuit = true;
            }
            else if (ev.key.key == SDLK_P) {
                paused = !paused;
            }
        }
        else if (ev.type == SDL_EVENT_GAMEPAD_ADDED) {
            const bool alreadyOpen = std::any_of(pads.begin(), pads.end(), [&](const GamepadPtr& pad) {
                return pad != nullptr && SDL_GetGamepadID(pad.get()) == ev.gdevice.which;
            });
            if (!alreadyOpen) {
                if (SDL_Gamepad* pad = SDL_OpenGamepad(ev.gdevice.which)) {
                    pads.emplace_back(pad);
                }
            }
        }
        else if (ev.type == SDL_EVENT_GAMEPAD_REMOVED) {
            auto it = std::remove_if(pads.begin(), pads.end(), [&](const GamepadPtr& pad) {
                return pad != nullptr && SDL_GetGamepadID(pad.get()) == ev.gdevice.which;
            });
            pads.erase(it, pads.end());
        }
        else if (ev.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
            if (ev.gbutton.button == SDL_GAMEPAD_BUTTON_START) {
                paused = !paused;
            }
        }
    }

    void shutdown() {
        pads.clear();
        keyState = nullptr;
    }
};

InputManager::InputManager()
    : impl_(std::make_unique<Impl>()) {
}

InputManager::~InputManager() {
    shutdown();
}

void InputManager::shutdown() {
    if (impl_) {
        impl_->shutdown();
    }
}

void InputManager::pollEvents() {
    impl_->syncGamepads();

    SDL_Event event{};
    while (SDL_PollEvent(&event)) {
        impl_->processEvent(event);
        if (impl_->shouldQuit) {
            break;
        }
    }

    SDL_UpdateGamepads();
    impl_->keyState = SDL_GetKeyboardState(nullptr);
}

bool InputManager::isActionDown(GameAction action) const {
    constexpr float kDeadzone = 0.25f;
    const bool* keys = impl_->keyState;

    switch (action) {
    case GameAction::MoveLeft:
        if (keys && (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A])) {
            return true;
        }
        for (const auto& pad : impl_->pads) {
            if (SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_DPAD_LEFT) ||
                (SDL_GetGamepadAxis(pad.get(), SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f) < -kDeadzone) {
                return true;
            }
        }
        return false;

    case GameAction::MoveRight:
        if (keys && (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D])) {
            return true;
        }
        for (const auto& pad : impl_->pads) {
            if (SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_DPAD_RIGHT) ||
                (SDL_GetGamepadAxis(pad.get(), SDL_GAMEPAD_AXIS_LEFTX) / 32767.0f) > kDeadzone) {
                return true;
            }
        }
        return false;

    case GameAction::MoveUp:
        if (keys && (keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W])) {
            return true;
        }
        for (const auto& pad : impl_->pads) {
            if (SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_DPAD_UP) ||
                (SDL_GetGamepadAxis(pad.get(), SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f) < -kDeadzone) {
                return true;
            }
        }
        return false;

    case GameAction::MoveDown:
        if (keys && (keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S])) {
            return true;
        }
        for (const auto& pad : impl_->pads) {
            if (SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_DPAD_DOWN) ||
                (SDL_GetGamepadAxis(pad.get(), SDL_GAMEPAD_AXIS_LEFTY) / 32767.0f) > kDeadzone) {
                return true;
            }
        }
        return false;

    case GameAction::Fire:
        if (keys && (keys[SDL_SCANCODE_SPACE] || keys[SDL_SCANCODE_Z])) {
            return true;
        }
        for (const auto& pad : impl_->pads) {
            // SOUTH = A on Xbox, Cross (X) on PlayStation
            if (SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_SOUTH) ||
                SDL_GetGamepadButton(pad.get(), SDL_GAMEPAD_BUTTON_WEST) ||
                (SDL_GetGamepadAxis(pad.get(), SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) / 32767.0f) > kDeadzone) {
                return true;
            }
        }
        return false;
    }

    return false;
}

bool InputManager::quitRequested() const noexcept {
    return impl_ && impl_->shouldQuit;
}

bool InputManager::isPaused() const noexcept {
    return impl_ && impl_->paused;
}

void InputManager::setPaused(bool paused) noexcept {
    if (impl_) {
        impl_->paused = paused;
    }
}
