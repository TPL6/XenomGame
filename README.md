# 2D Game Engine & Vertical Xenon 2000

A reusable C++20 2D Game Engine (`engine`) built with **SDL3**, **OpenGL 3.3 Core** (via GLAD), **GLM**, and **Box2D 3.2**, together with a vertical space-shooter clone of **Xenon 2000: Project PCF** (`xenon2000`).

---

## Project Structure

```text
.
├── CMakeLists.txt               # Root: C++20, packages, warnings, ASan option, add_subdirectory()
├── CMakePresets.json            # Presets: default, asan, xcode (configure) + debug, release, asan (build)
├── vcpkg.json                   # vcpkg manifest (SDL3, Box2D 3.2.0, GLM, GLAD OpenGL 3.3 + baseline)
├── .github/workflows/build.yml  # CI: clean-clone Windows + Linux portability builds
├── graphics/                    # Xenon 2000: Project PCF BMP sprite sheets and backgrounds
├── engine/                      # Target 1: Generic 2D Game Engine static library
│   ├── CMakeLists.txt
│   ├── include/engine/          # Public engine API (zero SDL3, OpenGL, or Box2D headers exposed)
│   ├── src/                     # Private backend, RAII wrappers, and EntryPoint.cpp (owns main())
│   └── shaders/                 # External GLSL 330 core shaders copied at build time
└── game/                        # Target 2: Vertical Xenon 2000 executable (links only `engine`)
    ├── CMakeLists.txt
    ├── include/
    └── src/                     # XenonGame.cpp (Game subclass + createGame() factory), actors, level
```

### Architecture Highlights

- **Strict Target Separation & Encapsulation**: `engine` links `SDL3::SDL3`, `glad::glad`, and `box2d::box2d` as `PRIVATE` dependencies. The `xenon2000` executable links only `engine` and cannot include SDL3, OpenGL, or Box2D headers directly.
- **Engine-Owned Entry Point (Inversion of Control)**: `main()` lives inside `engine/src/EntryPoint.cpp` with centralized exception handling and MSVC Debug CRT leak checks. Each game implements a `Game` subclass (`config()`, `setup()`, `createFirstLevel()`) and defines `createGame()` (see `game/src/XenonGame.cpp`).
- **RAII Resource Management**: Dedicated RAII wrappers own `SDL_Init`/`SDL_Quit` (`SDLContext`), `SDL_Window` (`Window`), `SDL_GLContext` + GLAD (`GLContext`), VAO/VBO/EBO (`VertexArray`, `VertexBuffer`, `IndexBuffer`), `Shader`, `Texture`, `b2WorldId` (`Box2DWorld`), `b2BodyId` (`PhysicsBody`), and `SDL_Gamepad` handles.
- **Renderer Abstraction**: Actors submit draw descriptors to an abstract `Renderer` interface implemented by `GLRenderer` using OpenGL 3.3 Core, indexed quads (`VAO` + `VBO` + `EBO`), `glm::ortho` pixel-space projection, and per-sprite TRS model matrices.
- **Fixed-Timestep Game Loop**: Logic and Box2D simulation run at a fixed 60 Hz step (`1/60 s`, 4 sub-steps), strictly separated from rendering.
- **Box2D 3.2 Collisions**: MKS units (`32 px = 1 m`), kinematic sensor hitboxes, dynamic bullet projectiles (`isBullet = true`), and category/mask collision filtering (`Player`, `Enemy`, `PlayerShot`, `EnemyShot`).

---

## Prerequisites

- **C++20 compiler** (Visual Studio 2022+ on Windows, Xcode / Clang on macOS, or GCC/Clang on Linux)
- **CMake** 3.25 or newer
- **Git**
- **vcpkg** installed locally with the `VCPKG_ROOT` environment variable set

### Installing vcpkg (once per machine)

**Windows (PowerShell):**
```powershell
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
setx VCPKG_ROOT "C:\vcpkg"
```

**macOS / Linux:**
```bash
cd ~
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
./bootstrap-vcpkg.sh
echo 'export VCPKG_ROOT="$HOME/vcpkg"' >> ~/.zshrc   # or ~/.bashrc
```

Restart your terminal/IDE after setting `VCPKG_ROOT`.

---

## Build & Run

From a clean clone of the repository:

### Windows (PowerShell)
```powershell
cmake --preset default
cmake --build --preset debug
.\build\game\Debug\xenon2000.exe
```

### macOS (Xcode)
```bash
cmake --preset xcode
open build-xcode/vertical_xenon_2000.xcodeproj
```

### macOS / Linux (Terminal)
```bash
cmake --preset default -DCMAKE_BUILD_TYPE=Release
cmake --build --preset release
./build/game/xenon2000
```

> CMake automatically installs all dependencies from `vcpkg.json` and copies `shaders/` and `graphics/` next to the compiled executable. Relative paths resolve against `SDL_GetBasePath()`, so the executable runs from any working directory.

---

## Checking Memory (CRT Debug Heap & AddressSanitizer)

- **Windows Debug builds** automatically enable the MSVC Debug CRT heap leak detector (`_CrtSetDbgFlag`) in `EntryPoint.cpp`.
- **AddressSanitizer (all platforms)**:
  ```powershell
  cmake --preset asan
  cmake --build --preset asan
  .\build-asan\game\Debug\xenon2000.exe
  ```
  *(On MSVC, CMake automatically copies `clang_rt.asan_dynamic-x86_64.dll` next to `xenon2000.exe` so it runs directly from the terminal.)*

---

## Controls

| Action | Keyboard | Xbox Controller | PlayStation Controller |
| :--- | :--- | :--- | :--- |
| **Move** | Arrow Keys / `W A S D` | Left Stick / D-Pad | Left Stick / D-Pad |
| **Fire Missile** | `Space` / `Z` | `A` (or `X` / RT) | `Cross (X)` (or `Square` / R2) |
| **Pause** | `P` | `Start` / `Menu` | `Options` |
| **Quit** | `Esc` | — | — |
