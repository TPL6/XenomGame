#pragma once

#include <SDL3/SDL.h>
#include <filesystem>
#include <string>
#include <string_view>

// Resolves a relative asset/shader path against the executable's directory (SDL_GetBasePath)
// first so launching from any working directory works reliably, then falls back to CWD.
inline std::string resolveAssetPath(std::string_view relativePath) {
    namespace fs = std::filesystem;

    const fs::path relative(relativePath);
    std::error_code ec;

    if (const char* basePath = SDL_GetBasePath(); basePath != nullptr && *basePath != '\0') {
        const fs::path fromExe = fs::path(basePath) / relative;
        if (fs::exists(fromExe, ec)) {
            return fromExe.string();
        }
    }

    if (fs::exists(relative, ec)) {
        return relative.string();
    }

    return std::string(relativePath);
}
