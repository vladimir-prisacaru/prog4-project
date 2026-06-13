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
        RegisterParameter("game_mode", &GameManager::m_GameModeStr);
    }

    void GameManager::OnInit(EngineCtx& ctx)
    {
        m_SceneManager = ctx.sceneManager;
        m_EventManager = ctx.eventManager;

        if (m_GameModeStr == "versus")
            m_GameMode = GameMode::Versus;
        else if (m_GameModeStr == "coop")
            m_GameMode = GameMode::Coop;
        else
            m_GameMode = GameMode::Solo;

        m_EventManager->AddListener(GameEvent::PlayerDied, this);
        m_EventManager->AddListener(GameEvent::EnemyDied, this);
        m_EventManager->AddListener(GameEvent::EnemySpawned, this);

        if (m_CurrentLevel == nullptr)
            LoadNewLevel();
    }

    void GameManager::Update(EngineCtx& ctx)
    {
        switch (m_CurrentState)
        {
            case State::Normal:
                if (m_Players.empty())
                {
                    m_Players = ctx.sceneManager->GetAllComponentsByType<Player>();

                    for (auto& player : m_Players)
                    {
                        const int id { player->GetId() };

                        // First ever load: create fresh data
                        if (m_PlayerData.find(id) == m_PlayerData.end())
                        {
                            PlayerData data { };
                            data.alive = true;
                            data.score = 0;
                            data.lives = m_MaxPlayerLives;

                            m_PlayerData[id] = data;
                        }
                        else
                        {
                            // New level load: reset lives, carry score
                            m_PlayerData[id].alive = true;
                            m_PlayerData[id].lives = m_MaxPlayerLives;
                        }
                    }

                    // Broadcast current state so UI reflects reality on every level load
                    for (auto& player : m_Players)
                    {
                        const int id { player->GetId() };
                        const auto& data { m_PlayerData[id] };

                        m_EventManager->QueueEvent(Event { GameEvent::ScoreUpdated,
                            m_GameMode == GameMode::Versus ? id : -1, data.score });

                        m_EventManager->QueueEvent(Event { GameEvent::LivesUpdated,
                            m_GameMode == GameMode::Versus ? id : -1, data.lives });
                    }

                    return;
                }
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
            case State::LoadLevel:
                if (m_LoadTimer >= m_LoadDelay)
                {
                    LoadNewLevel();

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

    void GameManager::OnDestroy(EngineCtx& ctx)
    {
        m_EventManager->RemoveListener(GameEvent::PlayerDied, this);
        m_EventManager->RemoveListener(GameEvent::EnemyDied, this);
        m_EventManager->RemoveListener(GameEvent::EnemySpawned, this);
    }

    void GameManager::Notify(const Event& event)
    {
        switch (event.id)
        {
            case GameEvent::PlayerDied:
                OnPlayerDied(event.value1, event.value2);
                break;
            case GameEvent::EnemyDied:
                OnEnemyDied(event.value1, event.value2);
                break;
            case GameEvent::EnemySpawned:
                m_EnemyCount = m_EnemyCount < 0 ? 1 : m_EnemyCount + 1;
                break;
        }
    }

    void GameManager::OnPlayerDied(int id, int killerId)
    {
        if (m_CurrentState != State::Normal)
            return;

        auto it { m_PlayerData.find(id) };

        if (it == m_PlayerData.end())
        {
            logError("Unknown player died (id={})", id);

            return;
        }

        it->second.alive = false;

        if (m_GameMode == GameMode::Versus)
        {
            // Award points to the killer if killed by another player
            if (killerId >= 0 && killerId != id)
            {
                auto killerIt { m_PlayerData.find(killerId) };

                if (killerIt != m_PlayerData.end())
                {
                    const int killScore { 500 };
                    killerIt->second.score += killScore;

                    m_EventManager->QueueEvent(Event { GameEvent::ScoreUpdated,
                        killerId, killerIt->second.score });
                }
            }

            it->second.lives -= 1;

            m_EventManager->QueueEvent(Event { GameEvent::LivesUpdated,
                id, it->second.lives });

            if (it->second.lives <= 0)
            {
                // Find the surviving player
                int winnerId { -1 };
                for (auto& [pid, data] : m_PlayerData)
                {
                    if (pid != id)
                    {
                        winnerId = pid;
                        break;
                    }
                }

                m_EventManager->QueueEvent(Event { GameEvent::GameWon, winnerId });

                StartLoad();
            }
            else
            {
                StartReset();
            }
        }
        else
        {
            // Solo / Co-op: shared lives tracked via the first player's entry
            auto& sharedData { m_PlayerData.begin()->second };
            sharedData.lives -= 1;

            m_EventManager->QueueEvent(Event { GameEvent::LivesUpdated,
                -1, sharedData.lives });

            if (sharedData.lives <= 0)
            {
                m_EventManager->QueueEvent(Event { GameEvent::GameOver });

                StartLoad();
            }
            else
            {
                StartReset();
            }
        }
    }

    void GameManager::OnEnemyDied(int playerId, int points)
    {
        auto it { m_PlayerData.find(playerId) };

        if (it != m_PlayerData.end())
        {
            it->second.score += points;

            m_EventManager->QueueEvent(Event { GameEvent::ScoreUpdated,
                m_GameMode == GameMode::Versus ? playerId : -1, it->second.score });
        }
        else
            logError("Unknown player (id={}) killed an enemy", playerId);

        // Check if last enemy
        if (m_EnemyCount - 1 <= 0)
            OnLastEnemyDied();
        else
            m_EnemyCount--;
    }

    void GameManager::OnLastEnemyDied()
    {
        if (m_CurrentState != State::Normal)
            return;

        if (m_GameMode != GameMode::Versus)
        {
            // Solo: the one player wins; Co-op: both players win together
            m_EventManager->QueueEvent(Event { GameEvent::GameWon, -1 });

            StartLoad();
        }
    }

    void GameManager::StartReset()
    {
        m_ResetTimer = 0.0f;
        m_CurrentState = State::LevelReset;

        m_EnemyCount = -1;
        m_EventManager->QueueEvent(Event { GameEvent::LevelReset });
    }

    void GameManager::StartLoad()
    {
        m_LoadTimer = 0.0f;
        m_CurrentState = State::LoadLevel;

        // Clear player refs now to avoid dangling pointers after scene unload.
        // Keep m_PlayerData so scores carry across levels.
        m_Players.clear();

        m_EnemyCount = -1;
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

        if (m_CurrentLevel != nullptr)
            m_SceneManager->UnloadScene(m_CurrentLevel);

        if (m_Levels.size() < 2)
        {
            m_LastLevelId = 0;

            m_SceneManager->LoadScene(m_Levels[0], &m_CurrentLevel);

            m_CurrentState = State::Normal;

            return;
        }

        auto randomInt =
            [] (int min, int max) -> int
            {
                static std::mt19937 gen(std::random_device {}());
                std::uniform_int_distribution<int> dist(min, max);
                return dist(gen);
            };

        int levelId { m_LastLevelId };
        int attempts { 0 };
        const int maxAttempts { 100 };

        while (levelId == m_LastLevelId && attempts < maxAttempts)
        {
            levelId = randomInt(0, static_cast<int>(m_Levels.size()) - 1);
            attempts++;
        }

        if (attempts >= maxAttempts)
            levelId = 0;

        m_LastLevelId = levelId;

        m_SceneManager->LoadScene(m_Levels[levelId], &m_CurrentLevel);

        m_CurrentState = State::Normal;
    }
}