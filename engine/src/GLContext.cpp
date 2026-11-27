#include "GLContext.h"
#include "Window.h"

#include <cstring>
#include <glad/glad.h>
#include <iostream>

namespace {

    void* loadGLProc(const char* name) {
        const SDL_FunctionPointer proc = SDL_GL_GetProcAddress(name);
        void* address = nullptr;
        static_assert(sizeof(address) == sizeof(proc));
        std::memcpy(&address, &proc, sizeof(address));
        return address;
    }

} // namespace

GLContext::GLContext(const Window& window, bool vsync) {
    if (!window.isValid()) {
        std::cerr << "GLContext: window is invalid\n";
        return;
    }

    SDL_Window* nativeWindow = window.nativeHandle();
    context_ = SDL_GL_CreateContext(nativeWindow);
    if (context_ == nullptr) {
        std::cerr << "GLContext: SDL_GL_CreateContext failed: " << SDL_GetError() << "\n";
        return;
    }

    if (!SDL_GL_MakeCurrent(nativeWindow, context_)) {
        std::cerr << "GLContext: SDL_GL_MakeCurrent failed: " << SDL_GetError() << "\n";
        release();
        return;
    }

    if (!gladLoadGLLoader(loadGLProc)) {
        std::cerr << "GLContext: failed to initialize GLAD\n";
        release();
        return;
    }

    gladLoaded_ = true;
    SDL_GL_SetSwapInterval(vsync ? 1 : 0);
}

GLContext::~GLContext() {
    release();
}

GLContext::GLContext(GLContext&& other) noexcept
    : context_(other.context_)
    , gladLoaded_(other.gladLoaded_) {
    other.context_ = nullptr;
    other.gladLoaded_ = false;
}

GLContext& GLContext::operator=(GLContext&& other) noexcept {
    if (this != &other) {
        release();
        context_ = other.context_;
        gladLoaded_ = other.gladLoaded_;
        other.context_ = nullptr;
        other.gladLoaded_ = false;
    }
    return *this;
}

void GLContext::release() noexcept {
    if (context_ != nullptr) {
        SDL_GL_DestroyContext(context_);
        context_ = nullptr;
    }
    gladLoaded_ = false;
}
