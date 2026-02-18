#include <format>

#include "FPSCounter.h"

void dae::FPSCounter::Update(float deltaTime)
{
    if (m_TextComponent == nullptr)
    {
        auto text { m_Parent->GetComponent<TextComponent>() };

        if (text == nullptr)
            return;

        m_TextComponent = text;
    }

    float fps { 1.0f / deltaTime };

    if (m_PrintCounter >= m_PrintInterval)
    {
        const float averageFPS { m_SumFPS / m_NumFPS };

        m_TextComponent->SetText(std::format("FPS: {:.1f}", averageFPS));

        m_PrintCounter = 0.0f;
        m_SumFPS = 0.0f;
        m_NumFPS = 0;
    }
    else
    {
        m_PrintCounter += deltaTime;
        m_SumFPS += fps;
        m_NumFPS++;
    }
}