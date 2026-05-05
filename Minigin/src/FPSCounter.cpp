#include <format>

#include "FPSCounter.h"

void dae::FPSCounter::Update(float deltaTime)
{
    if (m_TextComponent == nullptr)
    {
        auto text { GetOwner()->GetComponent<TextComponent>() };

        if (text == nullptr)
            return;

        m_TextComponent = text;
    }

    m_AccTime += deltaTime;
    m_NumFrames++;

    if (m_AccTime >= m_PrintInterval)
    {
        const float averageFPS { 1.0f / (m_AccTime / m_NumFrames) };

        m_TextComponent->SetText(std::format("FPS: {:.1f}", averageFPS));

        m_AccTime = 0.0f;
        m_NumFrames = 0;
    }
}