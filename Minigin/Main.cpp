#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TextureComponent.h"
#include "FPSCounter.h"
#include "Scene.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
    #include <vld.h>
#endif

#include <filesystem>

namespace fs = std::filesystem;



static void load()
{
    auto& scene = dae::SceneManager::GetInstance().CreateScene();

    auto bgObj { std::make_unique<dae::GameObject>() };
    auto bgTextureComp { bgObj->AddComponent<dae::TextureComponent>() };
    bgTextureComp->SetTexture("background.png");
    scene.Add(std::move(bgObj));

    auto logoObj { std::make_unique<dae::GameObject>() };
    logoObj->SetPosition(358, 180);
    auto logoTextureComp { logoObj->AddComponent<dae::TextureComponent>() };
    logoTextureComp->SetTexture("logo.png");
    scene.Add(std::move(logoObj));

    auto defaultFont { dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36) };

    auto titleObject { std::make_unique<dae::GameObject>() };
    auto titleTextComp { titleObject->AddComponent<dae::TextComponent>() };
    titleTextComp->SetFont(defaultFont);
    titleTextComp->SetText("Programming 4 Assignment");
    titleTextComp->SetColor({ 255, 255, 0, 255 });
    titleObject->SetPosition(292, 20);
    scene.Add(std::move(titleObject));

    auto fpsObject { std::make_unique<dae::GameObject>() };
    fpsObject->SetPosition(20, 20);
    auto fpsTextComp { fpsObject->AddComponent<dae::TextComponent>() };
    fpsTextComp->SetFont(defaultFont);
    fpsTextComp->SetText("FPS: 0");
    fpsTextComp->SetColor({ 255, 255, 0, 255 });
    //auto fpsCounterComp { fpsObject->AddComponent<dae::FPSCounter>() };
    fpsObject->AddComponent<dae::FPSCounter>();
    scene.Add(std::move(fpsObject));
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

    dae::Minigin engine(data_location);

    engine.Run(load);

    return 0;
}