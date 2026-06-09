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

    /*
    Scene* scene { ctx.sceneManager->CreateScene() };
    auto defaultFont { ctx.resourceManager->LoadFont("Lingua.otf", 36) };
    
    // Background
    auto bgObj { scene->Instantiate() };
    bgObj->AddComponent<TextureComponent>()->SetTexture("background.png");
    
    // Logo
    auto logoObj { scene->Instantiate() };
    logoObj->GetTransform().SetLocalPos(358, 180);
    logoObj->AddComponent<TextureComponent>()->SetTexture("logo.png");
    
    // Title
    auto titleObject { scene->Instantiate() };
    titleObject->GetTransform().SetLocalPos(292, 20);
    titleObject->AddComponent<TextureComponent>();
    auto titleTextComp { titleObject->AddComponent<TextComponent>() };
    titleTextComp->SetFont(defaultFont);
    titleTextComp->SetText("Programming 4 Assignment");
    titleTextComp->SetColor({ 255, 255, 0, 255 });

    // FPS counter
    auto fpsObject { scene->Instantiate() };
    fpsObject->GetTransform().SetLocalPos(20, 20);
    fpsObject->AddComponent<TextureComponent>();
    auto fpsTextComp { fpsObject->AddComponent<TextComponent>() };
    fpsTextComp->SetFont(defaultFont);
    fpsTextComp->SetText("FPS: 0");
    fpsTextComp->SetColor({ 255, 255, 0, 255 });
    fpsObject->AddComponent<FPSCounter>();

    // Helper lambda to make a player
    auto makePlayer = [&](int playerIndex, float startX, float uiY,
        const std::string& texture)
        -> std::tuple<GameObject*, HealthComponent*, ScoreComponent*>
    {
        // Player character
        auto playerObj { scene->Instantiate() };
        playerObj->GetTransform().SetLocalPos(startX, 300);
        playerObj->AddComponent<TextureComponent>()->SetTexture(texture);

        auto* health { playerObj->AddComponent<HealthComponent>() };
        health->SetMaxLives(3);
        health->SetPlayerIndex(playerIndex);

        auto* score { playerObj->AddComponent<ScoreComponent>() };
        score->SetPlayerIndex(playerIndex);

        // Lives display
        auto livesObj { scene->Instantiate() };
        livesObj->GetTransform().SetLocalPos(10, uiY);
        livesObj->AddComponent<TextureComponent>();
        auto livesText { livesObj->AddComponent<TextComponent>() };
        livesText->SetFont(defaultFont);
        livesText->SetText(".");
        auto livesDisplay { livesObj->AddComponent<LivesDisplayComponent>() };
        livesDisplay->SetPlayerIndex(playerIndex);
        livesDisplay->SetMaxLives(3);

        // Score display
        auto scoreObj { scene->Instantiate() };
        scoreObj->GetTransform().SetLocalPos(200, uiY);
        scoreObj->AddComponent<TextureComponent>();
        auto scoreText { scoreObj->AddComponent<TextComponent>() };
        scoreText->SetFont(defaultFont);
        scoreText->SetText(".");
        scoreObj->AddComponent<PointsDisplayComponent>()->SetPlayerId(playerIndex);

        return { playerObj, health, score };
    };

    auto [player1, health1, score1] { makePlayer(0, 200.f, 100.f, "char1.png") };
    auto [player2, health2, score2] { makePlayer(1, 350.f, 140.f, "char2.png") };

    // Player 1 - controller
    InputManager& input { *ctx.inputManager };

    input.AddControllerCommand(0, ControllerButton::ButtonX, KeyState::Down,
        std::make_unique<DamageCommand>(health2));
    input.AddControllerCommand(0, ControllerButton::ButtonA, KeyState::Down,
        std::make_unique<GainPointsCommand>(score1, 100));

    input.AddControllerCommand(0, ControllerButton::DPadUp, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1, glm::vec3 { 0.0f, -1.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadDown, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1, glm::vec3 { 0.0f, 1.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadLeft, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1, glm::vec3 { -1.0f, 0.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadRight, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1, glm::vec3 { 1.0f, 0.0f, 0.0f }, 10.0f));

    // Player 2 - keyboard
    input.AddKeyboardCommand(SDL_SCANCODE_C, KeyState::Down,
        std::make_unique<DamageCommand>(health1));
    input.AddKeyboardCommand(SDL_SCANCODE_Z, KeyState::Down,
        std::make_unique<GainPointsCommand>(score2, 100));

    input.AddKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2, glm::vec3 { 0.0f, -1.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2, glm::vec3 { 0.0f, 1.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2, glm::vec3 { -1.0f, 0.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2, glm::vec3 { 1.0f, 0.0f, 0.0f }, 10.0f));

    // Sound test
    scene->Instantiate()->AddComponent<SoundTestComponent>();

    */
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