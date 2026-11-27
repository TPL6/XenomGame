#include "engine/Engine.h"
#include "engine/Game.h"

#include <exception>
#include <iostream>
#include <memory>

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main() {
#if defined(_MSC_VER) && defined(_DEBUG)
    // Report any heap allocations still live at program exit to both VS Output and stderr.
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
#endif

    try {
        // 1. Ask the game to describe itself. Declared BEFORE `engine`, so `game`
        //    outlives `engine` during RAII teardown.
        std::unique_ptr<Game> game = createGame();
        if (!game) {
            std::cerr << "Fatal error: createGame() returned nullptr\n";
            return 1;
        }

        // 2. Start the engine with the game's settings.
        Engine engine{ game->config() };

        // 3. Let the game configure the running engine before the first level enters.
        game->setup(engine);

        // 4. Hand over the first level and run the game loop until quit.
        return engine.run(game->createFirstLevel());
    }
    catch (const std::exception& error) {
        std::cerr << "Fatal error: " << error.what() << '\n';
    }
    return 1;
}
