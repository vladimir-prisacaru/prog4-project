#pragma once

#include <filesystem>
namespace fs = std::filesystem;

namespace dae
{
    class SceneManager;
    class InputManager;
    class EventManager;
    class Renderer;
    class ServiceLocator;
    class ResourceManager;
    class Scene;
    class Physics;

    /* Engine context struct */
    struct EngineCtx
    {
        SceneManager* sceneManager { };
        InputManager* inputManager { };
        EventManager* eventManager { };
        ResourceManager* resourceManager { };
        Renderer* renderer { };
        ServiceLocator* services { };
        Physics* physics { };

        // Path of the Data folder with all the assets
        fs::path dataPath { };

        // Set per scene to pass into object methods
        Scene* scene { };

        // Only set on update, 0.0f otherwise
        float deltaTime { };
    };
}