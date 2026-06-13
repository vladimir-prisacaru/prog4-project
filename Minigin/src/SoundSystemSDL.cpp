#include "SoundSystemSDL.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>



enum class RequestType
{
    Play,
    PlayIfNotPlaying,
    Stop
};

struct PlayRequest
{
    RequestType type { RequestType::Play };
    std::string name { };     // track key (and audio file name for Play/Stop)
    std::string audioName { }; // audio file name override for PlayIfNotPlaying
    float volume { };
};

class dae::SoundSystemSDL::Impl
{
    public:

    // Must be called on the main thread (MIX_CreateMixerDevice requirement)
    Impl(const fs::path& clipsPath)
        : m_ClipsPath(clipsPath), m_Running(true)
    {
        if (!MIX_Init())
            throw std::runtime_error(std::string("MIX_Init failed: ") + SDL_GetError());

        m_Mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

        if (!m_Mixer)
        {
            MIX_Quit();

            throw std::runtime_error(std::string("MIX_CreateMixerDevice failed: ") + SDL_GetError());
        }

        m_Thread = std::thread(&Impl::AudioThreadFunc, this);
    }

    void SetDataPath(const fs::path& dataPath)
    {
        m_DataPath = dataPath;
    }

    void Enqueue(RequestType type, std::string_view id, float volume,
        std::string_view audioName = "")
    {
        {
            std::lock_guard lock(m_Mutex);
            m_Queue.push({ type, std::string(id), std::string(audioName), volume });
        }

        m_CV.notify_one();
    }

    // Must be called on the main thread (MIX_DestroyMixer requirement)
    ~Impl()
    {
        // Signal the audio thread to stop and wait for it to exit
        {
            std::lock_guard lock(m_Mutex);
            m_Running = false;
        }

        m_CV.notify_one();
        m_Thread.join();

        for (auto& [name, track] : m_NamedTracks)
        {
            MIX_StopTrack(track, 0);
            MIX_DestroyTrack(track);
        }
        m_NamedTracks.clear();

        m_ActiveTracks.clear();

        MIX_DestroyMixer(m_Mixer);
        m_Mixer = nullptr;

        m_Cache.clear();

        MIX_Quit();
    }



    private:

    void AudioThreadFunc()
    {
        while (true)
        {
            PlayRequest req;

            {
                std::unique_lock lock(m_Mutex);
                m_CV.wait(lock, [this] { return !m_Queue.empty() || !m_Running; });

                if (!m_Running && m_Queue.empty())
                    return;

                req = std::move(m_Queue.front());
                m_Queue.pop();
            }

            switch (req.type)
            {
                case RequestType::Play:
                    PlayImmediate(req.name, req.volume);
                    break;
                case RequestType::PlayIfNotPlaying:
                    PlayIfNotPlayingImmediate(req.name, req.audioName, req.volume);
                    break;
                case RequestType::Stop:
                    StopImmediate(req.name);
                    break;
            }

            RemoveFinishedTracks();
        }
    }

    // Returns the named track for id, creating it if needed
    MIX_Track* GetOrCreateNamedTrack(const std::string& name)
    {
        auto it = m_NamedTracks.find(name);
        if (it != m_NamedTracks.end())
            return it->second;

        MIX_Track* track = MIX_CreateTrack(m_Mixer);
        if (!track)
            return nullptr;

        m_NamedTracks[name] = track;
        return track;
    }

    void PlayImmediate(const std::string& name, float volume)
    {
        MIX_Audio* audio = GetAudio(name);
        if (!audio)
            return;

        if (volume == 1.0f)
        {
            // Fast path: fire-and-forget, SDL3_mixer manages the track lifetime
            MIX_PlayAudio(m_Mixer, audio);
            return;
        }

        // Non-default volume: use a named track so we can set gain
        MIX_Track* track = GetOrCreateNamedTrack(name);
        if (!track)
            return;

        MIX_SetTrackAudio(track, audio);
        MIX_SetTrackGain(track, volume);
        MIX_PlayTrack(track, 0);
    }

    void PlayIfNotPlayingImmediate(const std::string& name, const std::string& audioName,
        float volume)
    {
        MIX_Track* track = GetOrCreateNamedTrack(name);
        if (!track)
            return;

        if (MIX_TrackPlaying(track))
            return;

        // Load audio by the base file name (audioName), key the track by name
        const std::string& fileKey { audioName.empty() ? name : audioName };
        MIX_Audio* audio = GetAudio(fileKey);
        if (!audio)
            return;

        MIX_SetTrackAudio(track, audio);
        MIX_SetTrackGain(track, volume);
        MIX_PlayTrack(track, 0);
    }

    void StopImmediate(const std::string& name)
    {
        auto it = m_NamedTracks.find(name);
        if (it == m_NamedTracks.end())
            return;

        MIX_StopTrack(it->second, 0);
    }

    // Destroy unnamed tracks that have finished playing
    // Called on the audio thread only
    void RemoveFinishedTracks()
    {
        auto it = m_ActiveTracks.begin();
        while (it != m_ActiveTracks.end())
        {
            if (MIX_GetTrackRemaining(*it) == 0)
            {
                MIX_DestroyTrack(*it);
                it = m_ActiveTracks.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    MIX_Audio* GetAudio(const std::string& name)
    {
        auto it = m_Cache.find(name);
        if (it != m_Cache.end())
            return it->second.get();

        fs::path filePath = FindClipFile(name);
        if (filePath.empty())
            return nullptr;

        MIX_Audio* audio = MIX_LoadAudio(m_Mixer, filePath.string().c_str(), true);
        if (!audio)
            return nullptr;

        auto [inserted, _] = m_Cache.emplace(name, AudioPtr(audio));
        return inserted->second.get();
    }

    /* Returns the first file in m_DataPath / m_ClipsPath whose stem matches `name` */
    fs::path FindClipFile(const std::string& name) const
    {
        std::error_code ec;
        for (const auto& entry : fs::directory_iterator(m_DataPath / m_ClipsPath, ec))
        {
            if (entry.path().stem() == name)
                return entry.path();
        }
        return { };
    }

    struct AudioDeleter
    {
        void operator()(MIX_Audio* a) const { MIX_DestroyAudio(a); }
    };

    using AudioPtr = std::unique_ptr<MIX_Audio, AudioDeleter>;

    fs::path m_ClipsPath;
    fs::path m_DataPath;
    MIX_Mixer* m_Mixer = nullptr;
    std::unordered_map<std::string, AudioPtr> m_Cache; // audio thread only
    std::unordered_map<std::string, MIX_Track*> m_NamedTracks; // audio thread only
    std::vector<MIX_Track*> m_ActiveTracks; // audio thread only (unnamed fire-and-forget)

    std::thread m_Thread;
    std::mutex m_Mutex;
    std::condition_variable m_CV;
    std::queue<PlayRequest> m_Queue; // guarded by m_Mutex
    bool m_Running; // guarded by m_Mutex
};



// ------------------------
// --- Public interface ---
// ------------------------

namespace dae
{
    SoundSystemSDL::SoundSystemSDL(const fs::path& clipsPath)
        : m_Impl(std::make_unique<Impl>(clipsPath))
    { }

    SoundSystemSDL::~SoundSystemSDL() = default;

    void SoundSystemSDL::SetDataPath(const fs::path& dataPath)
    {
        m_Impl->SetDataPath(dataPath);
    }

    void SoundSystemSDL::Play(std::string_view id, float volume)
    {
        m_Impl->Enqueue(RequestType::Play, id, volume);
    }

    void SoundSystemSDL::PlayIfNotPlaying(std::string_view trackId, std::string_view audioName,
        float volume)
    {
        m_Impl->Enqueue(RequestType::PlayIfNotPlaying, trackId, volume, audioName);
    }

    void SoundSystemSDL::StopSound(std::string_view id)
    {
        m_Impl->Enqueue(RequestType::Stop, id, 0.0f);
    }
}