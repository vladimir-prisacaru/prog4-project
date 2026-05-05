#include "Minigin.h"
#include "SceneManager.h"
#include "ResourceManager.h"
#include "TextComponent.h"
#include "TextureComponent.h"
#include "FPSCounter.h"
#include "RotatorComponent.h"
//#include "ThrashTheCacheDemo.h"
#include "InputManager.h"
#include "Scene.h"
#include "EventManager.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"
#include "LivesDisplayComponent.h"
#include "PointsDisplayComponent.h"
#include "SteamAchievementObserver.h"
#include "GameCommands.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#if _DEBUG && __has_include(<vld.h>)
    #include <vld.h>
#endif

#include <filesystem>
#include <tuple>

namespace fs = std::filesystem;
using namespace dae;



static void load()
{
    auto& scene = SceneManager::GetInstance().CreateScene();
    auto defaultFont { ResourceManager::GetInstance().LoadFont("Lingua.otf", 36) };

    // Background
    auto bgObj { std::make_unique<GameObject>() };
    auto bgTextureComp { bgObj->AddComponent<TextureComponent>() };
    bgTextureComp->SetTexture("background.png");
    scene.Add(std::move(bgObj));

    // Logo
    auto logoObj { std::make_unique<GameObject>() };
    logoObj->GetTransform().SetLocalPos(358, 180);
    auto logoTextureComp { logoObj->AddComponent<TextureComponent>() };
    logoTextureComp->SetTexture("logo.png");
    scene.Add(std::move(logoObj));

    // Title
    auto titleObject { std::make_unique<GameObject>() };
    titleObject->AddComponent<TextureComponent>();
    auto titleTextComp { titleObject->AddComponent<TextComponent>() };
    titleTextComp->SetFont(defaultFont);
    titleTextComp->SetText("Programming 4 Assignment");
    titleTextComp->SetColor({ 255, 255, 0, 255 });
    titleObject->GetTransform().SetLocalPos(292, 20);
    scene.Add(std::move(titleObject));

    // FPS counter
    auto fpsObject { std::make_unique<GameObject>() };
    fpsObject->AddComponent<TextureComponent>();
    fpsObject->GetTransform().SetLocalPos(20, 20);
    auto fpsTextComp { fpsObject->AddComponent<TextComponent>() };
    fpsTextComp->SetFont(defaultFont);
    fpsTextComp->SetText("FPS: 0");
    fpsTextComp->SetColor({ 255, 255, 0, 255 });
    fpsObject->AddComponent<FPSCounter>();
    scene.Add(std::move(fpsObject));

    // Scene graph demo
    //auto pivot { std::make_unique<dae::GameObject>() };
    //pivot->GetTransform().SetLocalPos(200, 300);
    //
    //auto char1 { std::make_unique<dae::GameObject>() };
    //auto char1Tex { char1->AddComponent<dae::TextureComponent>() };
    //char1Tex->SetTexture("char1.png");
    //auto char1Rot { char1->AddComponent<dae::RotatorComponent>() };
    //char1Rot->SetAngularSpeed(1.5f);
    //char1Rot->SetRotationRadius(40.0f);
    //
    //auto char2 { std::make_unique<dae::GameObject>() };
    //auto char2Tex { char2->AddComponent<dae::TextureComponent>() };
    //char2Tex->SetTexture("char2.png");
    //auto char2Rot { char2->AddComponent<dae::RotatorComponent>() };
    //char2Rot->SetAngularSpeed(-2.0f);
    //char2Rot->SetRotationRadius(80.0f);
    //
    //char1->SetParent(pivot.get());
    //char2->SetParent(char1.get());
    //
    //scene.Add(std::move(pivot));
    //scene.Add(std::move(char1));
    //scene.Add(std::move(char2));

    // Thrash the cache demo:
    //auto cacheObj { std::make_unique<dae::GameObject>() };
    //cacheObj->AddComponent<dae::ThrashTheCacheDemo>();
    //scene.Add(std::move(cacheObj));

    // Movement input demo:
    dae::InputManager& input { InputManager::GetInstance() };

    //auto character1 { std::make_unique<GameObject>() };
    //character1->GetTransform().SetLocalPos(200, 300);
    //auto character1Tex { character1->AddComponent<TextureComponent>() };
    //character1Tex->SetTexture("char1.png");
    //auto movement1 { character1->AddComponent<MovementComponent>() };
    //movement1->SetSpeed(10.0f);
    //input.AddControllerCommand(0, ControllerButton::DPadUp, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement1, glm::vec3 { 0.0f, -1.0f, 0.0f }));
    //input.AddControllerCommand(0, ControllerButton::DPadDown, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement1, glm::vec3 { 0.0f, 1.0f, 0.0f }));
    //input.AddControllerCommand(0, ControllerButton::DPadLeft, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement1, glm::vec3 { -1.0f, 0.0f, 0.0f }));
    //input.AddControllerCommand(0, ControllerButton::DPadRight, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement1, glm::vec3 { 1.0f, 0.0f, 0.0f }));
    //
    //auto character2 { std::make_unique<GameObject>() };
    //character2->GetTransform().SetLocalPos(300, 300);
    //auto character2Tex { character2->AddComponent<TextureComponent>() };
    //character2Tex->SetTexture("char2.png");
    //auto movement2 { character2->AddComponent<MovementComponent>() };
    //movement2->SetSpeed(10.0f);
    //input.AddKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement2, glm::vec3 { 0.0f, -1.0f, 0.0f }));
    //input.AddKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement2, glm::vec3 { 0.0f, 1.0f, 0.0f }));
    //input.AddKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement2, glm::vec3 { -1.0f, 0.0f, 0.0f }));
    //input.AddKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed,
    //    std::make_unique<MoveCommand>(movement2, glm::vec3 { 1.0f, 0.0f, 0.0f }));
    //
    //scene.Add(std::move(character1));
    //scene.Add(std::move(character2));
    
    // Helper lambda to make a player
    auto makePlayer = [&](int playerIndex, float startX, float uiY,
        const std::string& texture)
        -> std::tuple<std::unique_ptr<GameObject>, HealthComponent*, ScoreComponent*>
    {
        // Player character
        auto playerObj = std::make_unique<GameObject>();
        playerObj->GetTransform().SetLocalPos(startX, 300);
        playerObj->AddComponent<TextureComponent>()->SetTexture(texture);

        auto* health = playerObj->AddComponent<HealthComponent>();
        health->SetMaxLives(3);
        health->SetPlayerIndex(playerIndex);

        auto* score = playerObj->AddComponent<ScoreComponent>();
        score->SetPlayerIndex(playerIndex);

        // Lives display
        {
            auto livesObj = std::make_unique<GameObject>();
            livesObj->GetTransform().SetLocalPos(10, uiY);
            livesObj->AddComponent<TextureComponent>();

            auto livesText = livesObj->AddComponent<TextComponent>();
            livesText->SetFont(defaultFont);
            livesText->SetText(".");

            // For debugging removing listeners
            livesObj->AddComponent<LivesDisplayComponent>();
            livesObj->RemoveComponent<LivesDisplayComponent>();

            auto livesDisplay = livesObj->AddComponent<LivesDisplayComponent>();
            livesDisplay->SetPlayerIndex(playerIndex);
            livesDisplay->SetMaxLives(3);

            scene.Add(std::move(livesObj));
        }

        // Score display
        {
            auto scoreObj = std::make_unique<GameObject>();
            scoreObj->GetTransform().SetLocalPos(200, uiY);
            scoreObj->AddComponent<TextureComponent>();

            auto scoreText = scoreObj->AddComponent<TextComponent>();
            scoreText->SetFont(defaultFont);
            scoreText->SetText(".");

            scoreObj->AddComponent<PointsDisplayComponent>()->SetPlayerId(playerIndex);

            scene.Add(std::move(scoreObj));
        }

        return { std::move(playerObj), health, score };
    };



    auto [player1, health1, score1] = makePlayer(0, 200.f, 100.f, "char1.png");

    auto [player2, health2, score2] = makePlayer(1, 350.f, 140.f, "char2.png");

    // Player 1 – controller
    input.AddControllerCommand(0, ControllerButton::ButtonX, KeyState::Down,
        std::make_unique<DamageCommand>(health2));
    input.AddControllerCommand(0, ControllerButton::ButtonA, KeyState::Down,
        std::make_unique<GainPointsCommand>(score1, 100));

    input.AddControllerCommand(0, ControllerButton::DPadUp, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1.get(), glm::vec3 { 0.0f, -1.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadDown, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1.get(), glm::vec3 { 0.0f, 1.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadLeft, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1.get(), glm::vec3 { -1.0f, 0.0f, 0.0f }, 10.0f));
    input.AddControllerCommand(0, ControllerButton::DPadRight, KeyState::Pressed,
        std::make_unique<MoveCommand>(player1.get(), glm::vec3 { 1.0f, 0.0f, 0.0f }, 10.0f));

    // Player 2 – keyboard
    input.AddKeyboardCommand(SDL_SCANCODE_C, KeyState::Down,
        std::make_unique<DamageCommand>(health1));
    input.AddKeyboardCommand(SDL_SCANCODE_Z, KeyState::Down,
        std::make_unique<GainPointsCommand>(score2, 100));

    input.AddKeyboardCommand(SDL_SCANCODE_W, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2.get(), glm::vec3 { 0.0f, -1.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_S, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2.get(), glm::vec3 { 0.0f, 1.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_A, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2.get(), glm::vec3 { -1.0f, 0.0f, 0.0f }, 10.0f));
    input.AddKeyboardCommand(SDL_SCANCODE_D, KeyState::Pressed,
        std::make_unique<MoveCommand>(player2.get(), glm::vec3 { 1.0f, 0.0f, 0.0f }, 10.0f));

    scene.Add(std::move(player1));
    scene.Add(std::move(player2));

    auto steamAchievementObj { std::make_unique<GameObject>() };
    steamAchievementObj->AddComponent<SteamAchievementObserver>();

    scene.Add(std::move(steamAchievementObj));
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