#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TextureComponent.h"
#include "FPSCounter.h"
#include "InputManager.h"
#include "Scene.h"
#include "ServiceLocator.h"
#include "SoundSystemSDL.h"
#include "SpriteComponent.h"
#include "Player.h"
#include "TunnelComponent.h"
#include "GridComponent.h"
#include "GridRenderer.h"
#include "Enemy.h"
#include "GameManager.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
    #include <vld.h>
#endif

#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;
using namespace dae;



static void load(EngineCtx& ctx)
{
    ctx.services->RegisterSoundSystem(std::make_unique<SoundSystemSDL>("Data/audioclips"));

    ctx.sceneManager->LoadScene("scene0.xml");
}


int main(int, char* [])
{
    #if __EMSCRIPTEN__

    fs::path data_location = "";

    #else

    fs::path data_location = "./Data/";
    if (!fs::exists(data_location))
        data_location = "../Data/";

    #endif //__EMSCRIPTEN__

    {
        dae::Minigin engine(data_location);

        engine.Run(load);
    }

    return 0;
}