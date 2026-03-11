#include "MovementComponent.h"

void dae::MovementComponent::Move(glm::vec3 direction)
{
    GameObject* owner = GetOwner();

    if (owner == nullptr)
        return;

    auto& transform = owner->GetTransform();

    transform.SetLocalPos(
        transform.GetLocalPos().x + direction.x * m_Speed,
        transform.GetLocalPos().y + direction.y * m_Speed,
        0.f);
}

void dae::MovementComponent::SetSpeed(float speed)
{
    m_Speed = speed;
}



dae::MoveCommand::MoveCommand(MovementComponent* owner, glm::vec3 direction)
    : m_Owner { owner }, m_Direction { direction }
{ }

void dae::MoveCommand::Execute()
{
    m_Owner->Move(m_Direction);
}