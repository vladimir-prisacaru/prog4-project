#pragma once

#include "InputCommand.h"
#include "HealthComponent.h"
#include "ScoreComponent.h"

namespace dae
{
    class DamageCommand final : public InputCommand
    {
        public:

        explicit DamageCommand(HealthComponent* health)
            : m_Health(health)
        { }

        void Execute() override
        {
            if (m_Health)
                m_Health->TakeDamage();
        }

        private:
        HealthComponent* m_Health;
    };

    class GainPointsCommand final : public InputCommand
    {
        public:
        explicit GainPointsCommand(ScoreComponent* score, int points = 100)
            : m_Score(score), m_Points(points)
        { }

        void Execute() override
        {
            if (m_Score)
                m_Score->AddPoints(m_Points);
        }

        private:

        ScoreComponent* m_Score;
        int m_Points;
    };

    class MoveCommand final : public InputCommand
    {
        public:

        explicit MoveCommand(GameObject* owner, glm::vec3 direction, float speed) :
            m_Owner(owner), m_Direction(direction), m_Speed(speed)
        { }

        virtual ~MoveCommand() = default;

        void Execute() override
        {
            auto& transform = m_Owner->GetTransform();

            transform.SetLocalPos(
                transform.GetLocalPos().x + m_Direction.x * m_Speed,
                transform.GetLocalPos().y + m_Direction.y * m_Speed,
                0.f);
        }

        private:

        GameObject* m_Owner;
        glm::vec3 m_Direction;
        float m_Speed;
    };
}