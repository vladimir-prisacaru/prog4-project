#pragma once

#include "GameObject.h"
#include "EventManager.h"

namespace dae
{
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

        struct PlayerData
        {
            bool alive { };
            int score { };
        };

        enum class State
        {
            Normal,
            LevelReset,
            GameOver
        };

        std::vector<Player*> m_Players { };
        std::unordered_map<int, PlayerData> m_PlayerData { };

        void OnPlayerDied(int id);
        void OnEnemyDied(int playerId, int points);
        void OnLastEnemyDied();

        void ResetLevel();
        void LoadNewLevel();

        // Params
        std::vector<fs::path> m_Levels { };
        int m_MaxPlayerLives { };
        float m_LoadDelay { };
        float m_ResetDelay { };

        // Internal
        State m_CurrentState { State::Normal };
        Scene* m_CurrentLevel { };
        int m_CurrentLives { };
        int m_LastLevelId { };
        float m_LoadTimer { };
        float m_ResetTimer { };

        // Cached
        SceneManager* m_SceneManager { };
        EventManager* m_EventManager { };
    };
}