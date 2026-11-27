#define SDL_MAIN_HANDLED // The engine owns main() in EntryPoint.cpp; do not let SDL replace it.
#include <SDL3/SDL_main.h>

#include "SDLContext.h"

#include <iostream>

SDLContext::SDLContext(SDL_InitFlags flags) {
    SDL_SetMainReady();
    if (!SDL_Init(flags)) {
        std::cerr << "SDLContext: SDL_Init failed: " << SDL_GetError() << "\n";
        return;
    }
    initialized_ = true;
}

SDLContext::~SDLContext() {
    release();
}

SDLContext::SDLContext(SDLContext&& other) noexcept
    : initialized_(other.initialized_) {
    other.initialized_ = false;
}

SDLContext& SDLContext::operator=(SDLContext&& other) noexcept {
    if (this != &other) {
        release();
        initialized_ = other.initialized_;
        other.initialized_ = false;
    }
    return *this;
}

void SDLContext::release() noexcept {
    if (initialized_) {
        SDL_Quit();
        initialized_ = false;
    }
}
