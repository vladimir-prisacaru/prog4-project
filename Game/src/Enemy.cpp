#include "Enemy.h"
#include "Player.h"
#include "SceneManager.h"
#include "SpriteComponent.h"

#include <array>

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

        // Get sprite component (optional – enemy may not have one)
        m_Sprite = GetComponent<SpriteComponent>();

        // Set initial state
        m_PathfindTimer = m_PathfindFrequency;
    }

    void Enemy::Update(EngineCtx& ctx)
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

        // Only re-pathfind once we've either hit the timer OR the target changed,
        // AND we are not mid-way between two nodes (i.e. the current node has been reached).
        // This prevents re-starting the path from node 0 (closest tile) before the
        // enemy has cleared the node it was already heading toward, which caused back-and-forth.
        const bool timerFired { m_PathfindTimer >= m_PathfindFrequency };
        const bool targetChanged { m_TargetPlayer != closestPlayer };
        const bool atNodeBoundary { m_CurrentPath.nodes.empty() || m_NextPathNodeId == 0 };

        if ((timerFired || targetChanged) && (atNodeBoundary || targetChanged))
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
        HandleAnimations();
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

        if (m_NextPathNodeId < 0 || m_NextPathNodeId >= static_cast<int>(m_CurrentPath.nodes.size()))
        {
            logError("Invalid next node id '{}'", m_NextPathNodeId);

            return;
        }

        auto& transform { GetTransform() };
        const glm::vec2 oldPos { transform.GetWorldPos() };
        const glm::vec2 nodePos { m_CurrentPath.nodes[m_NextPathNodeId] };

        // --- NaN-proof direction ---
        // If the enemy is already sitting on (or very close to) the target node, snap and
        // advance immediately rather than trying to normalise a near-zero vector.
        const glm::vec2 toNode { nodePos - oldPos };
        const float toNodeLen { glm::length(toNode) };

        if (toNodeLen < 1e-4f)
        {
            // Already at node – snap and fall through to index update below.
            transform.SetLocalPos(nodePos);
        }
        else
        {
            const glm::vec2 moveDir { toNode / toNodeLen }; // safe normalise

            // Update last direction for animations
            m_LastDir = moveDir;

            const glm::vec2 displacement { moveDir * m_MoveSpeed * deltaTime };
            const glm::vec2 newPos { oldPos + displacement };

            const float oldProj { glm::dot(oldPos,   moveDir) };
            const float newProj { glm::dot(newPos,   moveDir) };
            const float nodeProj { glm::dot(nodePos,  moveDir) };

            const bool crossedNode { nodeProj >= oldProj && nodeProj <= newProj };

            if (!crossedNode)
            {
                transform.SetLocalPos(newPos);
                return;
            }

            // Snap to the node we just crossed.
            transform.SetLocalPos(nodePos);
        }

        // ---- Advance node index ----
        if (m_CurrentState == State::Wander)
        {
            // If there's a cached path, we are still navigating to its start (node 0).
            if (!m_CachedWanderPath.nodes.empty())
            {
                // Reached the end of the bridging path – activate the cached wander path.
                if (const int nextId { m_NextPathNodeId + 1 };
                    nextId >= static_cast<int>(m_CurrentPath.nodes.size()))
                {
                    m_CurrentPath = std::move(m_CachedWanderPath);
                    m_CachedWanderPath = Path { };
                    m_NextPathNodeId = 0;
                }
                // Not there yet, keep walking the bridge.
                else
                {
                    m_NextPathNodeId++;
                }
            }
            // Actively wandering along the cached path.
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
                        nextId >= static_cast<int>(m_CurrentPath.nodes.size()))
                    {
                        m_NextPathNodeId = static_cast<int>(m_CurrentPath.nodes.size()) - 2;
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
            // Reached the end of the seek path.
            if (const int nextId { m_NextPathNodeId + 1 };
                nextId >= static_cast<int>(m_CurrentPath.nodes.size()))
            {
                m_CurrentPath = Path { };
                m_NextPathNodeId = 0;
            }
            else
            {
                m_NextPathNodeId++;
            }
        }
    }

    void Enemy::HandleAnimations()
    {
        if (m_Sprite == nullptr)
            return;

        static const std::array<std::string, 4> idleNames {
            "idle_up", "idle_right", "idle_down", "idle_left"
        };

        static const std::array<std::string, 4> moveNames {
            "move_up", "move_right", "move_down", "move_left"
        };

        const int dirIdx { GetDirInt(m_LastDir) };

        switch (m_CurrentState)
        {
            case State::Idle:
                m_Sprite->SetAnimationIfChanged(idleNames[dirIdx]);
                break;
            case State::Wander:
            case State::Seek:
                m_Sprite->SetAnimationIfChanged(moveNames[dirIdx]);
                break;
            case State::Attack:
                // No attack animation defined yet; stay on last move animation.
                break;
        }
    }

    int Enemy::GetDirInt(glm::vec2 dir) const
    {
        if (glm::length(dir) < 0.01f)
            return 1; // default: right

        dir = glm::normalize(dir);

        const glm::vec2 u { 0.0f, -1.0f };
        const glm::vec2 r { 1.0f,  0.0f };
        const glm::vec2 d { 0.0f,  1.0f };
        const glm::vec2 l { -1.0f,  0.0f };

        float bestDot { glm::dot(dir, u) };
        int   bestIdx { 0 };

        const float dotR { glm::dot(dir, r) };
        if (dotR > bestDot) { bestDot = dotR; bestIdx = 1; }

        const float dotD { glm::dot(dir, d) };
        if (dotD > bestDot) { bestDot = dotD; bestIdx = 2; }

        const float dotL { glm::dot(dir, l) };
        if (dotL > bestDot) { bestIdx = 3; }

        return bestIdx;
    }
}