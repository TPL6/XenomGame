#include "VerticalXenonLevel.h"
#include "engine/Game.h"

#include <memory>

namespace {

    class XenonGame final : public Game {
    public:
        EngineConfig config() const override {
            EngineConfig cfg;
            cfg.title = "Vertical Xenon 2000";
            cfg.width = 640;
            cfg.height = 480;
            cfg.vsync = true;
            return cfg;
        }

        std::unique_ptr<ILevel> createFirstLevel() override {
            const EngineConfig cfg = config();
            return std::make_unique<VerticalXenonLevel>(
                static_cast<float>(cfg.width),
                static_cast<float>(cfg.height)
            );
        }
    };

} // namespace

// Factory required by the engine's EntryPoint.cpp.
std::unique_ptr<Game> createGame() {
    return std::make_unique<XenonGame>();
}
