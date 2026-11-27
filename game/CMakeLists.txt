add_executable(xenon2000
    include/CollisionLayers.h
    include/Enemy.h
    include/EnemyProjectile.h
    include/Explosion.h
    include/LonerEnemy.h
    include/Missile.h
    include/PlayerShip.h
    include/RusherEnemy.h
    include/VerticalXenonLevel.h

    src/Enemy.cpp
    src/EnemyProjectile.cpp
    src/Explosion.cpp
    src/LonerEnemy.cpp
    src/Missile.cpp
    src/PlayerShip.cpp
    src/RusherEnemy.cpp
    src/VerticalXenonLevel.cpp
    src/XenonGame.cpp
)

target_include_directories(xenon2000 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Only links the engine: SDL3, OpenGL (GLAD), and Box2D headers are not visible here.
# main() is provided by engine/src/EntryPoint.cpp and calls createGame() from src/XenonGame.cpp.
target_link_libraries(xenon2000 PRIVATE engine)

enable_warnings(xenon2000)

# Copy engine shaders (plus MSVC ASan DLL when enabled) and game assets next to the executable.
engine_add_runtime_files(xenon2000)

if (EXISTS ${CMAKE_SOURCE_DIR}/graphics)
    add_custom_command(TARGET xenon2000 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_SOURCE_DIR}/graphics
                $<TARGET_FILE_DIR:xenon2000>/graphics
    )
endif()

set_target_properties(xenon2000 PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:xenon2000>"
    XCODE_SCHEME_WORKING_DIRECTORY "$<TARGET_FILE_DIR:xenon2000>"
)
