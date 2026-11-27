cmake_minimum_required(VERSION 3.25)
project(vertical_xenon_2000 LANGUAGES C CXX)

# C++20 strictly (no compiler-specific extensions) across all targets.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Locate dependencies provided by vcpkg in manifest mode.
find_package(SDL3 3.4 CONFIG REQUIRED)
find_package(box2d CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Reusable compiler warning helper called by every target.
function(enable_warnings target)
    if (MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# AddressSanitizer option (applied before add_subdirectory so engine and games are both instrumented).
option(ENGINE_ENABLE_ASAN "Build with AddressSanitizer" OFF)
if (ENGINE_ENABLE_ASAN)
    if (MSVC)
        # /RTC (enabled by default in Debug) conflicts with /fsanitize=address.
        string(REGEX REPLACE "/RTC[^ ]*" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
        add_compile_options(/fsanitize=address /Zi)
        add_link_options(/INCREMENTAL:NO)
    else()
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

add_subdirectory(engine)
add_subdirectory(game)

# In Visual Studio, F5 runs the game executable instead of ALL_BUILD or the static library.
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT xenon2000)
