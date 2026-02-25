#include "Minigin.h"
#include "InputManager.h"
#include "SceneManager.h"
#include "Renderer.h"
#include "ResourceManager.h"

#include <stdexcept>
#include <sstream>
#include <thread>
#include <iostream>

#if WIN32
    #define WIN32_LEAN_AND_MEAN
    #define NOMINMAX
    #include <windows.h>
#endif // WIN32

#include <SDL3/SDL.h>
//#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>



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

    Renderer::GetInstance().Init(g_Window);

    ResourceManager::GetInstance().Init(dataPath);
}

dae::Minigin::~Minigin()
{
    Renderer::GetInstance().Destroy();

    SDL_DestroyWindow(g_Window);

    g_Window = nullptr;

    SDL_Quit();
}

void dae::Minigin::Run(const std::function<void()>& load)
{
    load();

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



    m_Quit = !InputManager::GetInstance().ProcessInput();

    while (m_FixedUpdateLag >= FIXED_TIMESTEP)
    {
        SceneManager::GetInstance().FixedUpdate(FIXED_TIMESTEP);

        m_FixedUpdateLag -= FIXED_TIMESTEP;
    }

    SceneManager::GetInstance().Update(deltaTime);

    Renderer::GetInstance().Render();



    // Wait until the end of capped frame time

    const auto frameTime { std::chrono::duration_cast<std::chrono::nanoseconds>(
        seconds(FRAME_TIME)) };

    const auto targetTime { frameStartTime + frameTime };



    // This approach was too inaccurate (error of ~4FPS)
    // sleep_for and sleep_until are not accurate enough for frame capping

    //std::this_thread::sleep_until(targetTime);



    // This approach is exactly accurate, but not optimized ("busy waiting")
    while (clock::now() < targetTime)
    {
        std::this_thread::yield();
    }



    // This approach was too inaccurate (error of ~1FPS)
    // An attempt to have a course phase with sleep_for and a fine phase with busy waiting

    //using milliseconds = std::chrono::milliseconds;
    //auto now { clock::now() };
    //
    //while (now < targetTime)
    //{
    //    auto remaining = targetTime - now;
    //
    //    if (remaining > milliseconds(1))
    //        std::this_thread::sleep_for(milliseconds(1)); // coarse phase
    //    else
    //        std::this_thread::yield(); // fine phase
    //
    //    now = clock::now();
    //}
}