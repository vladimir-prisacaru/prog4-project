#pragma once

#include "GameObject.h"
#include "InputCommand.h"

namespace dae
{
    class MovementComponent : public Component
    {
        public:

        explicit MovementComponent(GameObject* owner) : Component(owner) { }
        ~MovementComponent() override = default;

        void Move(glm::vec3 direction);

        void SetSpeed(float speed);

        private:
        float m_Speed { 100.f };
    };

    class MoveCommand final : public InputCommand
    {
        public:

        explicit MoveCommand(MovementComponent* owner, glm::vec3 direction);
        virtual ~MoveCommand() = default;

        void Execute() override;

        private:

        MovementComponent* m_Owner;
        glm::vec3 m_Direction;
    };
}