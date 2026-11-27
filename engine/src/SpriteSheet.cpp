#include "engine/SpriteSheet.h"

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <unordered_map>

std::vector<UVRect> makeSpriteSheetUVs(int columns, int rows, int maxFrames) {
    if (columns <= 0 || rows <= 0) {
        std::cerr << "Invalid grid size for sprite sheet UVs.\n";
        return {};
    }

    const int totalCells = columns * rows;
    const int count = (maxFrames > 0) ? std::min(maxFrames, totalCells) : totalCells;

    const float uStep = 1.0f / static_cast<float>(columns);
    const float vStep = 1.0f / static_cast<float>(rows);

    std::vector<UVRect> frames;
    frames.reserve(static_cast<std::size_t>(count));

    for (int r = 0; r < rows && static_cast<int>(frames.size()) < count; ++r) {
        for (int c = 0; c < columns && static_cast<int>(frames.size()) < count; ++c) {
            const float u0 = static_cast<float>(c) * uStep;
            const float v0 = static_cast<float>(r) * vStep;
            frames.push_back(UVRect{
                u0,
                v0,
                u0 + uStep,
                v0 + vStep
            });
        }
    }

    return frames;
}

std::vector<UVRect> makeSpriteSheetUVs(const Texture& texture, int columns, int rows, int maxFrames) {
    if (!texture.isValid()) {
        std::cerr << "Invalid texture for sprite sheet UVs.\n";
        return {};
    }
    return makeSpriteSheetUVs(columns, rows, maxFrames);
}

std::shared_ptr<const std::vector<UVRect>> getSpriteSheetUVs(int columns, int rows, int maxFrames) {
    static std::unordered_map<std::uint64_t, std::shared_ptr<const std::vector<UVRect>>> cache;

    const int totalCells = (columns > 0 && rows > 0) ? (columns * rows) : 0;
    const int effectiveCount = (maxFrames > 0) ? std::min(maxFrames, totalCells) : totalCells;

    const std::uint64_t key =
        (static_cast<std::uint64_t>(static_cast<std::uint16_t>(columns)) << 48) |
        (static_cast<std::uint64_t>(static_cast<std::uint16_t>(rows)) << 32) |
        static_cast<std::uint32_t>(effectiveCount);

    if (const auto it = cache.find(key); it != cache.end()) {
        return it->second;
    }

    auto frames = std::make_shared<const std::vector<UVRect>>(
        makeSpriteSheetUVs(columns, rows, effectiveCount)
    );
    cache.emplace(key, frames);
    return frames;
}
