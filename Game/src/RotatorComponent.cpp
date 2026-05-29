#include "RotatorComponent.h"

void dae::RotatorComponent::Update(float deltaTime)
{
    Transform& transform = GetOwner()->GetTransform();

    m_Angle += m_AngularSpeed * deltaTime;

    glm::vec3 unitVector { std::cosf(m_Angle), std::sinf(m_Angle), 0.0f };

    transform.SetLocalPos(unitVector * m_Radius);
}

void dae::RotatorComponent::SetAngularSpeed(float speed)
{
    m_AngularSpeed = speed;
}

void dae::RotatorComponent::SetRotationRadius(float radius)
{
    m_Radius = radius;
}