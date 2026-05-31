#pragma once

namespace dae
{
    class SceneManager;
    class InputManager;
    class EventManager;
    class Renderer;
    class ServiceLocator;
    class ResourceManager;
    class Scene;

    /* Engine context struct */
    struct EngineCtx
    {
        SceneManager* sceneManager { };
        InputManager* inputManager { };
        EventManager* eventManager { };
        ResourceManager* resourceManager { };
        Renderer* renderer { };
        ServiceLocator* services { };
        // Set per scene to pass into object methods
        Scene* scene { };
        // Only set on update
        float deltaTime { };
    };
}