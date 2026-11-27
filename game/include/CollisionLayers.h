#pragma once

#include <cstdint>

namespace CollisionLayers {
    inline constexpr std::uint64_t Player     = 1ULL << 0;
    inline constexpr std::uint64_t Enemy      = 1ULL << 1;
    inline constexpr std::uint64_t PlayerShot = 1ULL << 2;
    inline constexpr std::uint64_t EnemyShot  = 1ULL << 3;
} // namespace CollisionLayers
