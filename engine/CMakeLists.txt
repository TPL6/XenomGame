add_library(engine STATIC
    # Public API headers (no SDL3, OpenGL/GLAD, or Box2D headers exposed)
    include/engine/Actor.h
    include/engine/Animator.h
    include/engine/Engine.h
    include/engine/Game.h
    include/engine/ILevel.h
    include/engine/InputManager.h
    include/engine/Level.h
    include/engine/PhysicsBody.h
    include/engine/PhysicsTypes.h
    include/engine/PhysicsWorld.h
    include/engine/Renderer.h
    include/engine/ResourceManager.h
    include/engine/Shader.h
    include/engine/SpriteSheet.h
    include/engine/Texture.h

    # Private implementation (the only place SDL3, OpenGL/GLAD, and Box2D appear)
    src/Actor.cpp
    src/Animator.cpp
    src/AssetPath.h
    src/Engine.cpp
    src/EntryPoint.cpp
    src/GLBuffers.cpp
    src/GLBuffers.h
    src/GLContext.cpp
    src/GLContext.h
    src/GLRenderer.cpp
    src/GLRenderer.h
    src/InputManager.cpp
    src/Level.cpp
    src/PhysicsWorld.cpp
    src/ResourceManager.cpp
    src/SDLContext.cpp
    src/SDLContext.h
    src/Shader.cpp
    src/SpriteSheet.cpp
    src/Texture.cpp
    src/Window.cpp
    src/Window.h

    # Listed so they appear in IDEs; compiled by OpenGL at run time
    shaders/sprite.vert
    shaders/sprite.frag
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# SDL3, Box2D, and GLAD are strictly PRIVATE: game targets cannot include their headers.
target_link_libraries(engine
    PUBLIC
        glm::glm
    PRIVATE
        SDL3::SDL3
        box2d::box2d
        glad::glad
)

enable_warnings(engine)

set(ENGINE_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders CACHE INTERNAL "")

# Copies engine runtime files (GLSL shaders and MSVC ASan DLL when enabled) next to a game target.
function(engine_add_runtime_files target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${ENGINE_SHADER_DIR}
                $<TARGET_FILE_DIR:${target}>/shaders
    )

    if (MSVC AND ENGINE_ENABLE_ASAN)
        get_filename_component(MSVC_BIN_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${MSVC_BIN_DIR}/clang_rt.asan_dynamic-x86_64.dll"
                    $<TARGET_FILE_DIR:${target}>
        )
    endif()
endfunction()
