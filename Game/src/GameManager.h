#pragma once

#include "GameObject.h"
#include "EventManager.h"

namespace dae
{
    class Player;
    class ServiceLocator;

    class GameManager : public Component, public Registrar<GameManager>, public IObserver
    {
        public:

        static void Register();

        explicit GameManager(GameObject* owner) : Component(owner) { };
        ~GameManager() = default;

        void OnInit(EngineCtx& ctx) override;
        void Update(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        void Notify(const Event& event) override;

        private:

        enum class GameMode
        {
            Solo,
            Coop,
            Versus
        };

        struct PlayerData
        {
            bool alive { true };
            int score { };
            int lives { };
        };

        enum class State
        {
            Normal,
            LevelReset,
            LoadLevel
        };

        std::vector<Player*> m_Players { };
        std::unordered_map<int, PlayerData> m_PlayerData { };

        void OnPlayerDied(int id, int killerId);
        void OnEnemyDied(int playerId, int points);
        void OnLastEnemyDied();

        void StartReset();
        void StartLoad();
        void ResetLevel();
        void LoadNewLevel();

        // Params
        std::vector<fs::path> m_Levels { };
        int m_MaxPlayerLives { };
        float m_LoadDelay { };
        float m_ResetDelay { };
        std::string m_GameModeStr { };

        // Internal
        GameMode m_GameMode { GameMode::Solo };
        State m_CurrentState { State::Normal };
        Scene* m_CurrentLevel { };
        int m_LastLevelId { };
        float m_LoadTimer { };
        float m_ResetTimer { };
        int m_EnemyCount { -1 };

        // Cached
        SceneManager* m_SceneManager { };
        EventManager* m_EventManager { };
        ServiceLocator* m_Services { };
    };
}