#include "GameManager.h"
#include "SceneManager.h"
#include "Player.h"
#include "Enemy.h"

#include <random>

namespace dae
{
    void GameManager::Register()
    {
        RegisterComponent<GameManager>("game_manager");

        RegisterParameter("levels", &GameManager::m_Levels);
        RegisterParameter("max_player_lives", &GameManager::m_MaxPlayerLives);
        RegisterParameter("reset_delay", &GameManager::m_ResetDelay);
        RegisterParameter("load_delay", &GameManager::m_LoadDelay);
    }

    void GameManager::OnInit(EngineCtx& ctx)
    {
        m_SceneManager = ctx.sceneManager;
        m_EventManager = ctx.eventManager;

        m_EventManager->AddListener(GameEvent::PlayerDied, this);
        m_EventManager->AddListener(GameEvent::EnemyDied, this);
    }

    void GameManager::Update(EngineCtx & ctx)
    {
        if (m_Players.empty())
        {
            // Find players
            m_Players = ctx.sceneManager->GetAllComponentsByType<Player>();

            if (!m_PlayerData.empty())
                m_PlayerData.clear();

            for (auto& player : m_Players)
            {
                m_PlayerData[player->GetId()] = PlayerData { };
            }

            return;
        }

        switch (m_CurrentState)
        {
            case State::Normal:
                break;
            case State::LevelReset:
                if (m_ResetTimer >= m_ResetDelay)
                {
                    ResetLevel();

                    m_CurrentState = State::Normal;

                    return;
                }
                else
                {
                    m_ResetTimer += ctx.deltaTime;
                }
                break;
            case State::GameOver:
                if (m_LoadTimer >= m_LoadDelay)
                {
                    LoadNewLevel();

                    m_CurrentState = State::Normal;

                    return;
                }
                else
                {
                    m_LoadTimer += ctx.deltaTime;
                }
                break;
            default:
                break;
        }
    }

    void GameManager::OnDestroy(EngineCtx & ctx)
    {
        m_EventManager->RemoveListener(GameEvent::PlayerDied, this);
        m_EventManager->RemoveListener(GameEvent::EnemyDied, this);
    }

    void GameManager::Notify(const Event& event)
    {
        switch (event.id)
        {
            case GameEvent::PlayerDied:
                OnPlayerDied(event.value1);
                break;
            case GameEvent::EnemyDied:
                OnEnemyDied(event.value1, event.value2);
                break;
        }
    }

    void GameManager::OnPlayerDied(int id)
    {
        auto it { m_PlayerData.find(id) };

        if (it == m_PlayerData.end())
        {
            logError("Uknown player died (id={})", id);

            return;
        }

        it->second.alive = false;

        const int newLives { m_CurrentLives - 1 };

        if (newLives <= 0)
        {
            m_EventManager->QueueEvent(Event { GameEvent::GameOver });

            m_CurrentState = State::GameOver;
        }
        else
        {
            m_EventManager->QueueEvent(Event { GameEvent::LevelReset });

            m_CurrentState = State::LevelReset;
        }
    }

    void GameManager::OnEnemyDied(int playerId, int points)
    {
        auto it { m_PlayerData.find(playerId) };

        if (it != m_PlayerData.end())
        {
            it->second.score += points;

            m_EventManager->QueueEvent(Event { GameEvent::ScoreUpdated,
                playerId, it->second.score });
        }
        else
            logError("Uknown player (id={}) killed an enemy", playerId);

        // Check if last enemy
        auto enemies = m_SceneManager->GetAllObjectsByType<Enemy>();

        if (enemies.size() <= 0)
            OnLastEnemyDied();
    }

    void GameManager::OnLastEnemyDied()
    {
        m_EventManager->QueueEvent(Event { GameEvent::GameWon });

        m_CurrentState = State::GameOver;
    }

    void GameManager::ResetLevel()
    {
        m_ResetTimer = 0.0f;

        for (auto& player : m_Players)
        {
            player->Reset();
        }

        auto enemies { m_SceneManager->GetAllComponentsByType<Enemy>() };

        for (auto& enemy : enemies)
        {
            enemy->Reset();
        }
    }

    void GameManager::LoadNewLevel()
    {
        m_LoadTimer = 0.0f;

        if (m_Levels.empty())
            return;

        m_SceneManager->UnloadScene(m_CurrentLevel);

        if (m_Levels.size() < 2)
        {
            m_LastLevelId = 0;

            m_SceneManager->LoadScene(m_Levels[0]);

            return;
        }

        auto randomInt =
            [] (int min, int max) -> int
            {
                static std::mt19937 gen(std::random_device {}());
                std::uniform_int_distribution<int> dist(min, max);
                return dist(gen);
            };

        int levelId { -1 };
        int attempts { 0 };
        const int maxAttempts { 100 };

        while (levelId == m_LastLevelId || attempts < maxAttempts)
        {
            levelId = randomInt(0, m_Levels.size() - 1);
            attempts++;
        }

        if (levelId == -1 || attempts >= maxAttempts)
        {
            m_LastLevelId = 0;

            m_SceneManager->LoadScene(m_Levels[0]);

            return;
        }

        m_SceneManager->LoadScene(m_Levels[levelId], &m_CurrentLevel);
    }
}