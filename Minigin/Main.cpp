#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TextureComponent.h"
#include "FPSCounter.h"
#include "RotatorComponent.h"
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
    auto defaultFont { dae::ResourceManager::GetInstance().LoadFont("Lingua.otf", 36) };

    // Background
    auto bgObj { std::make_unique<dae::GameObject>() };
    auto bgTextureComp { bgObj->AddComponent<dae::TextureComponent>() };
    bgTextureComp->SetTexture("background.png");
    scene.Add(std::move(bgObj));

    // Logo
    auto logoObj { std::make_unique<dae::GameObject>() };
    logoObj->GetTransform().SetLocalPos(358, 180);
    auto logoTextureComp { logoObj->AddComponent<dae::TextureComponent>() };
    logoTextureComp->SetTexture("logo.png");
    scene.Add(std::move(logoObj));

    // Title
    auto titleObject { std::make_unique<dae::GameObject>() };
    titleObject->AddComponent<dae::TextureComponent>();
    auto titleTextComp { titleObject->AddComponent<dae::TextComponent>() };
    titleTextComp->SetFont(defaultFont);
    titleTextComp->SetText("Programming 4 Assignment");
    titleTextComp->SetColor({ 255, 255, 0, 255 });
    titleObject->GetTransform().SetLocalPos(292, 20);
    scene.Add(std::move(titleObject));

    // FPS counter
    auto fpsObject { std::make_unique<dae::GameObject>() };
    fpsObject->AddComponent<dae::TextureComponent>();
    fpsObject->GetTransform().SetLocalPos(20, 20);
    auto fpsTextComp { fpsObject->AddComponent<dae::TextComponent>() };
    fpsTextComp->SetFont(defaultFont);
    fpsTextComp->SetText("FPS: 0");
    fpsTextComp->SetColor({ 255, 255, 0, 255 });
    fpsObject->AddComponent<dae::FPSCounter>();
    scene.Add(std::move(fpsObject));

    // Scene graph demo
    auto pivot { std::make_unique<dae::GameObject>() };
    pivot->GetTransform().SetLocalPos(200, 300);

    auto char1 { std::make_unique<dae::GameObject>() };
    auto char1Tex { char1->AddComponent<dae::TextureComponent>() };
    char1Tex->SetTexture("char1.png");
    auto char1Rot { char1->AddComponent<dae::RotatorComponent>() };
    char1Rot->SetAngularSpeed(1.5f);
    char1Rot->SetRotationRadius(40.0f);

    auto char2 { std::make_unique<dae::GameObject>() };
    auto char2Tex { char2->AddComponent<dae::TextureComponent>() };
    char2Tex->SetTexture("char2.png");
    auto char2Rot { char2->AddComponent<dae::RotatorComponent>() };
    char2Rot->SetAngularSpeed(-2.0f);
    char2Rot->SetRotationRadius(80.0f);

    char1->SetParent(pivot.get());
    char2->SetParent(char1.get());

    scene.Add(std::move(pivot));
    scene.Add(std::move(char1));
    scene.Add(std::move(char2));
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