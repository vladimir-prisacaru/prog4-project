#include "Minigin.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "EventManager.h"
#include "ServiceLocator.h"

#include <stdexcept>
#include <sstream>
#include <thread>
#include <iostream>

#if WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif // WIN32

#if USE_STEAMWORKS
    #pragma warning (push)
    #pragma warning (disable:4996)
    #include <steam_api.h>
    #pragma warning (pop)
#endif // USE_STEAMWORKS

#include <SDL3/SDL.h>
//#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>



// ---------------
// --- Helpers ---
// ---------------

SDL_Window* g_Window { };

static void LogSDLVersion(const std::string& message, int major, int minor, int patch)
{
    #if WIN32

    std::stringstream ss;
    ss << message << major << "." << minor << "." << patch << "\n";
    OutputDebugString(ss.str().c_str());

    #else

    std::cout << message << major << "." << minor << "." << patch << "\n";

    #endif // WIN32
}

#ifdef __EMSCRIPTEN__

#include "emscripten.h"

void LoopCallback(void* arg)
{
    static_cast<dae::Minigin*>(arg)->RunOneFrame();
}

#endif // _EMSCRIPTEN__

static void PrintSDLVersion()
{
    LogSDLVersion("Compiled with SDL", SDL_MAJOR_VERSION,
        SDL_MINOR_VERSION, SDL_MICRO_VERSION);

    int version = SDL_GetVersion();

    LogSDLVersion("Linked with SDL ", SDL_VERSIONNUM_MAJOR(version),
        SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));

    // LogSDLVersion("Compiled with SDL_image ",SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_MICRO_VERSION);
    // version = IMG_Version();
    // LogSDLVersion("Linked with SDL_image ", SDL_VERSIONNUM_MAJOR(version), SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));

    LogSDLVersion("Compiled with SDL_ttf ", SDL_TTF_MAJOR_VERSION,
        SDL_TTF_MINOR_VERSION, SDL_TTF_MICRO_VERSION);

    version = TTF_Version();

    LogSDLVersion("Linked with SDL_ttf ", SDL_VERSIONNUM_MAJOR(version),
        SDL_VERSIONNUM_MINOR(version), SDL_VERSIONNUM_MICRO(version));
}



// ---------------
// --- Minigin ---
// ---------------

dae::Minigin::Minigin(const std::filesystem::path& dataPath)
{
    PrintSDLVersion();

    if (!SDL_InitSubSystem(SDL_INIT_VIDEO))
    {
        SDL_Log("Renderer error: %s", SDL_GetError());
        throw std::runtime_error(std::string("SDL_Init Error: ") + SDL_GetError());
    }

    g_Window = SDL_CreateWindow(
        "Programming 4 assignment",
        1024,
        576,
        SDL_WINDOW_OPENGL
    );

    if (g_Window == nullptr)
    {
        throw std::runtime_error(std::string("SDL_CreateWindow Error: ") + SDL_GetError());
    }

    m_SceneManager = std::make_unique<SceneManager>(dataPath / "scenes");
    m_EventManager = std::make_unique<EventManager>();
    m_InputManager = std::make_unique<InputManager>();
    m_ResourceManager = std::make_unique<ResourceManager>(dataPath);
    m_Renderer = std::make_unique<Renderer>(g_Window);
    m_Services = std::make_unique<ServiceLocator>(dataPath);

    m_Context.sceneManager = m_SceneManager.get();
    m_Context.eventManager = m_EventManager.get();
    m_Context.inputManager = m_InputManager.get();
    m_Context.resourceManager = m_ResourceManager.get();
    m_Context.renderer = m_Renderer.get();
    m_Context.services = m_Services.get();

    m_SceneManager->m_Ctx = m_Context;
    m_ResourceManager->m_Ctx = m_Context;

#if USE_STEAMWORKS
    if (!SteamAPI_Init())
        throw std::runtime_error(
            std::string("Fatal Error - Steam must be running to play this game"
                " (SteamAPI_Init() failed)."));
    m_SteamInitialized = true;
#endif
}

dae::Minigin::~Minigin()
{
    m_Renderer.reset();
    m_Services.reset();

    #if USE_STEAMWORKS
    if (m_SteamInitialized)
        SteamAPI_Shutdown();
    #endif

    SDL_DestroyWindow(g_Window);

    g_Window = nullptr;

    SDL_Quit();
}

void dae::Minigin::Run(const std::function<void(EngineCtx& ctx)>& load)
{
    load(m_Context);

    #ifndef __EMSCRIPTEN__

    while (!m_Quit)
        RunOneFrame();

    #else

    emscripten_set_main_loop_arg(&LoopCallback, this, 0, true);

    #endif
}



void dae::Minigin::RunOneFrame()
{
    using clock = std::chrono::steady_clock;
    using seconds = std::chrono::duration<float>;



    const auto frameStartTime { clock::now() };

    const float deltaTime {
        seconds(frameStartTime - m_LastUpdateTime).count() };

    m_LastUpdateTime = frameStartTime;

    m_FixedUpdateLag += std::min(deltaTime, MAX_FRAME_TIME);



    m_Quit = !m_InputManager->ProcessInput();

#if USE_STEAMWORKS
    SteamAPI_RunCallbacks();
#endif

    m_Renderer->ImGuiNewFrame();

    while (m_FixedUpdateLag >= FIXED_TIMESTEP)
    {
        m_SceneManager->FixedUpdate(FIXED_TIMESTEP);

        m_FixedUpdateLag -= FIXED_TIMESTEP;
    }

    m_SceneManager->Update(deltaTime);

    m_EventManager->FlushEvents();

    m_Renderer->Render();



    // Wait until the end of capped frame time

    const auto frameTime { std::chrono::duration_cast<std::chrono::nanoseconds>(
        seconds(FRAME_TIME)) };

    const auto targetTime { frameStartTime + frameTime };

    // This approach is exactly accurate, but not optimized ("busy waiting")
    while (clock::now() < targetTime)
    {
        std::this_thread::yield();
    }
}