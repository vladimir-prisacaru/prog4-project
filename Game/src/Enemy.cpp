#include "Enemy.h"
#include "Player.h"
#include "SceneManager.h"

namespace dae
{
    void Enemy::Register()
    {
        RegisterComponent<Enemy>("enemy");

        RegisterParameter("pathfind_frequency", &Enemy::m_PathfindFrequency);
        RegisterParameter("move_speed", &Enemy::m_MoveSpeed);
    }

    void Enemy::OnInit(EngineCtx& ctx)
    {
        // Get all players
        m_Players = ctx.sceneManager->GetAllComponentsByType<Player>();

        // Get tunnel
        m_Tunnel = ctx.sceneManager->GetFirstComponentByType<TunnelComponent>();
        m_Graph = m_Tunnel->GetNavigationGraph();

        // Set initial state
        m_PathfindTimer = m_PathfindFrequency;
    }

    void Enemy::Update(EngineCtx & ctx)
    {
        // Get closest player
        Player* closestPlayer { };
        float closestDist { std::numeric_limits<float>::max() };

        for (auto& player : m_Players)
        {
            const float dist { glm::distance(player->GetTransform().GetWorldPos(),
                GetTransform().GetWorldPos()) };

            if (dist < closestDist)
            {
                closestPlayer = player;
                closestDist = dist;
            }
        }

        m_PathfindTimer += ctx.deltaTime;

        if (m_PathfindTimer >= m_PathfindFrequency || m_TargetPlayer != closestPlayer)
        {
            m_TargetPlayer = closestPlayer;
            m_PathfindTimer = 0.0f;

            if (PathfindToPlayer())
            {
                m_CurrentState = State::Seek;
            }
            else
            {
                if (m_CurrentState != State::Wander)
                {
                    if (FindWanderPath())
                        m_CurrentState = State::Wander;
                    else
                        m_CurrentState = State::Idle;
                }
            }
        }

        HandleMovement(ctx.deltaTime);
    }

    void Enemy::OnDestroy(EngineCtx&)
    {

    }

    void Enemy::OnOverlap(ICollider*)
    {

    }

    void Enemy::OnOverlapEnd(ICollider*)
    {

    }

    bool Enemy::PathfindToPlayer()
    {
        if (m_Tunnel == nullptr || m_Graph == nullptr || m_TargetPlayer == nullptr)
            return false;

        const auto startPos { GetTransform().GetWorldPos() };
        const auto endPos { m_TargetPlayer->GetTransform().GetWorldPos() };

        Path path { m_Graph->FindPath(startPos, endPos) };

        if (path.nodes.empty()) // failed to find path
            return false;

        m_CurrentPath = std::move(path);
        m_NextPathNodeId = 0;

        return true;
    }

    bool Enemy::FindWanderPath()
    {
        if (m_Tunnel == nullptr || m_Graph == nullptr)
            return false;

        const int searchDepth { 5 };

        Path path { m_Graph->GetConnectedPath(GetTransform().GetWorldPos(), searchDepth) };

        if (path.nodes.empty())
            return false;

        Path pathToNode0 { m_Graph->FindPath(GetTransform().GetWorldPos(), path.nodes[0]) };

        if (pathToNode0.nodes.empty())
            return false;

        m_CachedWanderPath = std::move(path);
        m_CurrentPath = std::move(pathToNode0);
        m_NextPathNodeId = 0;
        m_IsWanderingReversed = false;

        return true;
    }

    void Enemy::HandleMovement(float deltaTime)
    {
        if (m_CurrentState != State::Wander && m_CurrentState != State::Seek)
            return;

        if (m_CurrentPath.nodes.empty())
            return;

        if (m_NextPathNodeId < 0 || m_NextPathNodeId >= m_CurrentPath.nodes.size())
        {
            logError("Invalid next node id '{}'", m_NextPathNodeId);

            return;
        }

        auto& transform { GetTransform() };
        const glm::vec2 oldPos { transform.GetWorldPos() };
        const glm::vec2 nodePos { m_CurrentPath.nodes[m_NextPathNodeId] };
        const glm::vec2 moveDir { glm::normalize(nodePos - oldPos) };
        const glm::vec2 displacement { moveDir * m_MoveSpeed * deltaTime };
        const glm::vec2 newPos { oldPos + displacement };

        const float oldProj { glm::dot(oldPos, moveDir) };
        const float newProj { glm::dot(newPos, moveDir) };
        const float nodeProj { glm::dot(nodePos, moveDir) };

        const bool crossedNode { nodeProj >= oldProj && nodeProj <= newProj };

        if (!crossedNode)
        {
            transform.SetLocalPos(newPos);

            return;
        }

        // Update next node index
        if (m_CurrentState == State::Wander)
        {
            // If there's a cached path, check if its beggining was reached
            if (!m_CachedWanderPath.nodes.empty())
            {
                // Reached, move it to current path
                if (const int nextId { m_NextPathNodeId + 1 };
                    nextId >= m_CurrentPath.nodes.size())
                {
                    m_CurrentPath = std::move(m_CachedWanderPath);
                    m_CachedWanderPath = Path { };
                    m_NextPathNodeId = 0;
                }
                // Not reached, continue along
                else
                {
                    m_NextPathNodeId++;
                }
            }
            // No cached path, actively wandering
            else
            {
                if (m_IsWanderingReversed)
                {
                    if (const int nextId { m_NextPathNodeId - 1 };
                        nextId < 0)
                    {
                        m_NextPathNodeId = 1;
                        m_IsWanderingReversed = false;
                    }
                    else
                    {
                        m_NextPathNodeId--;
                    }
                }
                else
                {
                    if (const int nextId { m_NextPathNodeId + 1 };
                        nextId >= m_CurrentPath.nodes.size())
                    {
                        m_NextPathNodeId = m_CurrentPath.nodes.size() - 2;
                        m_IsWanderingReversed = true;
                    }
                    else
                    {
                        m_NextPathNodeId++;
                    }
                }
            }
        }
        else if (m_CurrentState == State::Seek)
        {
            // Reached the end of the path in seek
            if (const int nextId { m_NextPathNodeId + 1 };
                nextId >= m_CurrentPath.nodes.size())
            {
                m_CurrentPath = Path { };
                m_NextPathNodeId = 0;
            }
            else
            {
                m_NextPathNodeId++;
            }
        }

        transform.SetLocalPos(nodePos);
    }
}