#pragma once

#include <SDL3/SDL.h>

// RAII wrapper for SDL subsystem initialization and shutdown.
class SDLContext {
public:
    SDLContext() = default;
    explicit SDLContext(SDL_InitFlags flags);
    ~SDLContext();

    SDLContext(const SDLContext&) = delete;
    SDLContext& operator=(const SDLContext&) = delete;

    SDLContext(SDLContext&& other) noexcept;
    SDLContext& operator=(SDLContext&& other) noexcept;

    bool isValid() const noexcept { return initialized_; }

private:
    void release() noexcept;

    bool initialized_ = false;
};
