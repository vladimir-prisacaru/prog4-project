#include "MainMenu.h"
#include "TextComponent.h"

namespace dae
{
    static constexpr int OPTION_COUNT { 3 };

    static const char* k_OptionLabels[OPTION_COUNT]
    {
        "Play Solo",
        "Play Co-op",
        "Play Versus"
    };

    void MainMenu::Register()
    {
        RegisterComponent<MainMenu>("main_menu");

        RegisterParameter("solo_scene", &MainMenu::m_SoloScene);
        RegisterParameter("coop_scene", &MainMenu::m_CoopScene);
        RegisterParameter("versus_scene", &MainMenu::m_VersusScene);
    }

    void MainMenu::OnInit(EngineCtx& ctx)
    {
        m_SceneManager = ctx.sceneManager;
        m_OwnScene = ctx.scene;

        if (GetOwner()->GetChildCount() < 3)
            return;

        m_SoloText = GetOwner()->GetChildById(0)->GetComponent<TextComponent>();
        m_CoopText = GetOwner()->GetChildById(1)->GetComponent<TextComponent>();
        m_VersusText = GetOwner()->GetChildById(2)->GetComponent<TextComponent>();

        // Set up input
        auto* input { ctx.inputManager };

        input->AddKeyboardCommand(SDL_SCANCODE_UP,
            KeyState::Down, std::make_unique<MainMenuSwitchCommand>(this, false));

        input->AddKeyboardCommand(SDL_SCANCODE_DOWN,
            KeyState::Down, std::make_unique<MainMenuSwitchCommand>(this, true));

        input->AddKeyboardCommand(SDL_SCANCODE_RETURN,
            KeyState::Down, std::make_unique<MainMenuSelectCommand>(this));

        input->AddControllerCommand(0, ControllerButton::DPadUp,
            KeyState::Down, std::make_unique<MainMenuSwitchCommand>(this, false));

        input->AddControllerCommand(0, ControllerButton::DPadDown,
            KeyState::Down, std::make_unique<MainMenuSwitchCommand>(this, true));

        input->AddControllerCommand(0, ControllerButton::ButtonA,
            KeyState::Down, std::make_unique<MainMenuSelectCommand>(this));

        UpdateText();
    }

    void MainMenu::OnDestroy(EngineCtx& ctx)
    {
        auto* input { ctx.inputManager };

        input->RemoveKeyboardCommand(SDL_SCANCODE_UP, KeyState::Down);
        input->RemoveKeyboardCommand(SDL_SCANCODE_DOWN, KeyState::Down);
        input->RemoveKeyboardCommand(SDL_SCANCODE_RETURN, KeyState::Down);

        input->RemoveControllerCommand(0, ControllerButton::DPadUp, KeyState::Down);
        input->RemoveControllerCommand(0, ControllerButton::DPadDown, KeyState::Down);
        input->RemoveControllerCommand(0, ControllerButton::ButtonA, KeyState::Down);
    }

    void MainMenu::SwitchPrev()
    {
        m_CurrentSelection = (m_CurrentSelection - 1 + OPTION_COUNT) % OPTION_COUNT;
        UpdateText();
    }

    void MainMenu::SwitchNext()
    {
        m_CurrentSelection = (m_CurrentSelection + 1) % OPTION_COUNT;
        UpdateText();
    }

    void MainMenu::Select()
    {
        const std::string* scenes[OPTION_COUNT] { &m_SoloScene, &m_CoopScene, &m_VersusScene };

        const std::string& target { *scenes[m_CurrentSelection] };

        if (!target.empty())
            m_SceneManager->LoadScene(target);

        m_SceneManager->UnloadScene(m_OwnScene);
    }

    void MainMenu::UpdateText()
    {
        TextComponent* texts[OPTION_COUNT] { m_SoloText, m_CoopText, m_VersusText };

        for (int i { }; i < OPTION_COUNT; i++)
        {
            if (texts[i] == nullptr)
                continue;

            if (i == m_CurrentSelection)
                texts[i]->SetText(std::string(k_OptionLabels[i]) + " <");
            else
                texts[i]->SetText(k_OptionLabels[i]);
        }
    }

    void MainMenuSwitchCommand::Execute()
    {
        if (m_ToNext)
            m_Target->SwitchNext();
        else
            m_Target->SwitchPrev();
    }

    void MainMenuSelectCommand::Execute()
    {
        m_Target->Select();
    }
}