#pragma once

#include "GameObject.h"
#include "EventManager.h"

namespace dae
{
    class TextComponent;

    class PlayerUI final : public Component, public Registrar<PlayerUI>, public IObserver
    {
        public:

        static void Register();

        explicit PlayerUI(GameObject* owner) : Component(owner) { };
        ~PlayerUI() override = default;

        void OnInit(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        void Notify(const Event& event) override;

        private:

        int m_TargetPlayerId { };

        TextComponent* m_ScoreText { };
        TextComponent* m_LivesText { };
    };
}