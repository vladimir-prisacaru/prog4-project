#include "Enemy.h"
#include "Player.h"
#include "SceneManager.h"
#include "SpriteComponent.h"
#include "AttackComponent.h"
#include "EventManager.h"

#include <array>

namespace dae
{
    void Enemy::Register()
    {
        RegisterComponent<Enemy>("enemy");

        RegisterParameter("pathfind_frequency", &Enemy::m_PathfindFrequency);
        RegisterParameter("move_speed", &Enemy::m_MoveSpeed);
        RegisterParameter("attack_dirs", &Enemy::m_AttackDirs);
        RegisterParameter("attack_angle_threshold", &Enemy::m_AttackAngleThreshold);
        RegisterParameter("attack_range", &Enemy::m_AttackRange);
    }

    void Enemy::OnInit(EngineCtx& ctx)
    {
        m_Scene = ctx.scene;
        m_EventManager = ctx.eventManager;

        // Get all players
        m_Players = ctx.sceneManager->GetAllComponentsByType<Player>();

        // Get tunnel
        m_Tunnel = ctx.sceneManager->GetFirstComponentByType<TunnelComponent>();
        m_Graph = m_Tunnel->GetNavigationGraph();

        // Get sprite component (optional)
        m_Sprite = GetComponent<SpriteComponent>();

        // Get attack component (optional)
        m_Attack = GetOwner()->GetChildCount() > 0 ?
            GetOwner()->GetChildById(0)->GetComponent<AttackComponent>() : nullptr;

        // Fire pathfinding on the very first update
        m_PathfindTimer = m_PathfindFrequency;
    }

    void Enemy::Update(EngineCtx& ctx)
    {
        // Find closest player
        Player* closestPlayer { };
        float closestDist { std::numeric_limits<float>::max() };

        for (auto* player : m_Players)
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

        const bool timerFired { m_PathfindTimer >= m_PathfindFrequency };
        const bool targetChanged { m_TargetPlayer != closestPlayer };

        if (timerFired || targetChanged)
        {
            m_TargetPlayer = closestPlayer;
            m_PathfindTimer = 0.0f;

            if (PathfindToPlayer())
            {
                // Successfully found a path: enter/stay in Seek.
                // Clear the wander cache so it doesn't interfere if we later
                // fall back to wandering.
                m_CachedWanderPath = Path { };
                m_CurrentState = State::Seek;
            }
            else
            {
                // Can't reach player — wander if not already doing so.
                // We deliberately do NOT interrupt an in-progress wander, both
                // to avoid thrashing and because the existing path is still valid.
                if (m_CurrentState != State::Wander)
                {
                    if (FindWanderPath())
                        m_CurrentState = State::Wander;
                    else
                        m_CurrentState = State::Idle;
                }
            }
        }

        if (m_CurrentState != State::Attack)
        {
            if (TryAttack())
                m_CurrentState == State::Attack;
        }

        HandleMovement(ctx.deltaTime);
        HandleAttack();
        HandleDeath();
        HandleAnimations();
    }

    void Enemy::OnDestroy(EngineCtx&)
    { }

    void Enemy::OnOverlap(ICollider* other)
    {
        if (m_CurrentState == State::Dying || m_CurrentState == State::Dead)
            return;

        if (m_CurrentAttacker != nullptr)
            return;

        if (auto* comp { dynamic_cast<Component*>(other) };
            comp != nullptr)
        {
            if (auto* attack { comp->GetComponent<AttackComponent>() };
                attack != nullptr)
            {
                // If attack is from the player
                if (attack->IsFriendly())
                {
                    m_CurrentAttacker = attack;

                    m_CurrentState = State::Dying;

                    return;
                }
            }
        }
    }

    void Enemy::OnOverlapEnd(ICollider* other)
    {
        if (auto* comp { dynamic_cast<Component*>(other) };
            comp != nullptr)
        {
            if (auto* attack { comp->GetComponent<AttackComponent>() };
                attack != nullptr)
            {
                if (attack == m_CurrentAttacker)
                    m_CurrentAttacker = nullptr;
            }
        }
    }

    // Drops redundant path nodes
    static void TrimRedundantLeadingNodes(Path& path, glm::vec2 startPos)
    {
        // Collinearity threshold: cross-product magnitude must be below this
        constexpr float kCollinearEps { 0.5f };

        while (path.nodes.size() >= 2)
        {
            const glm::vec2 n0 { path.nodes[0] };
            const glm::vec2 n1 { path.nodes[1] };

            // Check collinearity of (startPos, n0, n1) via 2D cross product
            const glm::vec2 v0 { n0 - startPos };
            const glm::vec2 v1 { n1 - startPos };
            const float cross { v0.x * v1.y - v0.y * v1.x };

            if (std::abs(cross) > kCollinearEps)
                break; // corner, keep node[0]

            // Check that skipping node[0] is actually shorter
            const float dist01 { glm::length(n0 - startPos) + glm::length(n1 - n0) };
            const float distDirect { glm::length(n1 - startPos) };

            if (distDirect >= dist01 - kCollinearEps)
                break; // not shorter, keep node[0]

            path.nodes.erase(path.nodes.begin());
        }
    }

    bool Enemy::PathfindToPlayer()
    {
        if (m_Tunnel == nullptr || m_Graph == nullptr || m_TargetPlayer == nullptr)
            return false;

        const glm::vec2 startPos { GetTransform().GetWorldPos() };
        const glm::vec2 endPos { m_TargetPlayer->GetTransform().GetWorldPos() };

        Path path { m_Graph->FindPath(startPos, endPos) };

        if (path.nodes.empty())
            return false;

        TrimRedundantLeadingNodes(path, startPos);

        m_CurrentPath = std::move(path);
        m_NextPathNodeId = 0;

        return true;
    }

    bool Enemy::FindWanderPath()
    {
        if (m_Tunnel == nullptr || m_Graph == nullptr)
            return false;

        constexpr int searchDepth { 5 };

        Path path { m_Graph->GetConnectedPath(GetTransform().GetWorldPos(), searchDepth) };

        // Guard: need at least 2 nodes to bounce back and forth
        if (path.nodes.size() < 2)
            return false;

        const glm::vec2 startPos { GetTransform().GetWorldPos() };

        Path pathToNode0 { m_Graph->FindPath(startPos, path.nodes[0]) };

        if (pathToNode0.nodes.empty())
            return false;

        TrimRedundantLeadingNodes(pathToNode0, startPos);

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

        const glm::vec2 toNode { nodePos - oldPos };
        const float toNodeLen { glm::length(toNode) };

        if (toNodeLen < 1e-4f)
        {
            // Already at this node: snap and fall through to index update
            transform.SetLocalPos(nodePos);
        }
        else
        {
            const glm::vec2 moveDir { toNode / toNodeLen };
            const glm::vec2 displacement { moveDir * m_MoveSpeed * deltaTime };
            const glm::vec2 newPos { oldPos + displacement };

            // Update facing direction for animations.
            m_LastDir = moveDir;

            const float oldProj { glm::dot(oldPos,  moveDir) };
            const float newProj { glm::dot(newPos,  moveDir) };
            const float nodeProj { glm::dot(nodePos, moveDir) };

            const bool crossedNode { nodeProj >= oldProj && nodeProj <= newProj };

            if (!crossedNode)
            {
                transform.SetLocalPos(newPos);
                return;
            }

            // Snap to the node we just crossed
            transform.SetLocalPos(nodePos);
        }

        // ---- Advance node index ----
        if (m_CurrentState == State::Wander)
        {
            // Still on the bridge path that leads to the start of the wander route
            if (!m_CachedWanderPath.nodes.empty())
            {
                if (const int nextId { m_NextPathNodeId + 1 };
                    nextId >= static_cast<int>(m_CurrentPath.nodes.size()))
                {
                    // Reached the end of the bridge – activate the wander route
                    m_CurrentPath = std::move(m_CachedWanderPath);
                    m_CachedWanderPath = Path { };
                    m_NextPathNodeId = 0;
                }
                else
                {
                    m_NextPathNodeId++;
                }
            }
            // Actively wandering along the wander route
            else
            {
                if (m_IsWanderingReversed)
                {
                    if (const int nextId { m_NextPathNodeId - 1 }; nextId < 0)
                    {
                        // Bounced back to the start: go forward again from node 1
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
                        // Reached the far end: start reversing from the second-to-last node
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
            if (const int nextId { m_NextPathNodeId + 1 };
                nextId >= static_cast<int>(m_CurrentPath.nodes.size()))
            {
                // Reached the end of the seek path
                // Clear the path and transition to Idle so the enemy doesn't
                // freeze waiting for the re-pathfind timer
                m_CurrentPath = Path { };
                m_NextPathNodeId = 0;
                m_CurrentState = State::Idle;
            }
            else
            {
                m_NextPathNodeId++;
            }
        }
    }

    void Enemy::HandleAttack()
    {
        if (m_CurrentState != State::Attack)
            return;

        if (m_Sprite->IsAnimationFinished("attack"))
            m_CurrentState = State::Idle;
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

        static const std::array<std::string, 4> attackNames {
            "attack_up", "attack_right", "attack_down", "attack_left"
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
                m_Sprite->SetAnimationIfChanged(attackNames[dirIdx]);
                break;
            case State::Dying:
                m_Sprite->SetAnimationIfChanged("die");
                break;
            case State::Undying:
                m_Sprite->SetAnimationIfChanged("undie");
                break;
        }
    }

    void Enemy::HandleUndeath()
    {
        if (m_CurrentState != State::Undying)
            return;

        if (m_Sprite->IsAnimationFinished("undie"))
            m_CurrentState = State::Idle;
    }

    void Enemy::HandleDeath()
    {
        if (m_CurrentState != State::Dying)
            return;

        if (m_CurrentAttacker == nullptr)
        {
            m_CurrentState = State::Undying;

            return;
        }

        if (m_Sprite->IsAnimationFinished("die"))
            Die();
    }

    void Enemy::Die()
    {
        // handle death

        m_Scene->Destroy(GetOwner());

        int playerId { -1 };

        if (m_CurrentAttacker != nullptr)
        {
            auto* parent { m_CurrentAttacker->GetOwner()->GetParent() };

            if (auto* player { parent->GetComponent<Player>() })
                playerId = player->GetId();
        }

        m_EventManager->QueueEvent(Event { GameEvent::EnemyDied, playerId });
    }

    bool Enemy::TryAttack()
    {
        if (m_TargetPlayer == nullptr || m_Attack == nullptr)
            return false;

        const glm::vec2 pos { GetTransform().GetWorldPos() };
        const glm::vec2 playerPos { m_TargetPlayer->GetTransform().GetWorldPos() };

        const float distToPlayer { glm::length(playerPos - pos) };

        if (distToPlayer < 1e-5f || distToPlayer > m_AttackRange)
            return false;

        const glm::vec2 toPlayer { glm::normalize(playerPos - pos) };

        const float minDot { std::cos(glm::radians(m_AttackAngleThreshold)) };

        // If aligned
        for (auto& dir : m_AttackDirs)
        {
            const float dot = glm::dot(glm::normalize(dir), toPlayer);

            if (dot >= minDot)
            {
                m_Attack->StartAttacking(GetDirInt(dir));

                return true;
            }
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
        int bestIdx { 0 };

        const float dotR { glm::dot(dir, r) };
        if (dotR > bestDot) { bestDot = dotR; bestIdx = 1; }

        const float dotD { glm::dot(dir, d) };
        if (dotD > bestDot) { bestDot = dotD; bestIdx = 2; }

        const float dotL { glm::dot(dir, l) };
        if (dotL > bestDot) { bestIdx = 3; }

        return bestIdx;
    }
}