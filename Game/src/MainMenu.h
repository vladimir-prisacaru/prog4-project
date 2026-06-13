#pragma once

#include "GameObject.h"
#include "InputManager.h"
#include "SceneManager.h"

namespace dae
{
    class TextComponent;

    class MainMenu final : public Component, public Registrar<MainMenu>
    {
        public:

        static void Register();

        explicit MainMenu(GameObject* owner) : Component(owner) { };
        ~MainMenu() override = default;

        void OnInit(EngineCtx& ctx) override;
        void OnDestroy(EngineCtx& ctx) override;

        void SwitchPrev();
        void SwitchNext();
        void Select();

        private:

        void UpdateText();

        std::string m_SoloScene { };
        std::string m_CoopScene { };
        std::string m_VersusScene { };

        int m_CurrentSelection { };

        TextComponent* m_SoloText { };
        TextComponent* m_CoopText { };
        TextComponent* m_VersusText { };

        SceneManager* m_SceneManager { };
        Scene* m_OwnScene { };
    };

    class MainMenuSwitchCommand : public InputCommand
    {
        public:

        explicit MainMenuSwitchCommand(MainMenu* target, bool toNext) :
            m_Target(target), m_ToNext(toNext)
        { };

        void Execute() override;

        ~MainMenuSwitchCommand() override = default;

        private:

        MainMenu* m_Target { };
        bool m_ToNext { };
    };

    class MainMenuSelectCommand : public InputCommand
    {
        public:

        explicit MainMenuSelectCommand(MainMenu* target) :
            m_Target(target)
        { };

        void Execute() override;

        ~MainMenuSelectCommand() override = default;

        private:

        MainMenu* m_Target { };
    };
}