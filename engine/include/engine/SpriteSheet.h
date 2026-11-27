#pragma once

#include "engine/Renderer.h"
#include "engine/Texture.h"

#include <memory>
#include <vector>

// Builds a UV grid (columns x rows), row-major, top-left origin.
// If maxFrames > 0, stops after that many frames.
std::vector<UVRect> makeSpriteSheetUVs(int columns, int rows, int maxFrames = 0);
std::vector<UVRect> makeSpriteSheetUVs(const Texture& texture, int columns, int rows, int maxFrames = 0);

// Cached, shared UV grid: identical grids are built once and reused.
std::shared_ptr<const std::vector<UVRect>> getSpriteSheetUVs(int columns, int rows, int maxFrames = 0);
