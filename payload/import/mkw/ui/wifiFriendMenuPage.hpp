#pragma once

#include "import/cxx.hpp"
#include "multiMenuInputManager.hpp"
#include "sectionManager.hpp"
#include "yesNoPopupPage.hpp"

namespace mkw::UI
{

#if RMC

class WifiFriendMenuPage : public Page
{
public:
    void onActivate() override
    {
        LONGCALL void onActivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CF18, 0x80619C04, 0x8064C584, 0x8063B230));

        onActivate(this);

        // The payload is run after 'onInit' is called, so we set the handler
        // here.

        s_onOption = new (mkw::System::System::Instance().systemHeap(), 4)
            MenuInputManager::Handler<WifiFriendMenuPage>(
                this, &WifiFriendMenuPage::onOption
            );

        MultiControlInputManager* multiControlInputManager =
            reinterpret_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(
            MenuInputManager::InputType::Option, s_onOption
        );
    }

    void onDeactivate() override
    {
        LONGCALL void onDeactivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CFF8, 0x80619CE4, 0x8064C664, 0x8063B310));

        onDeactivate(this);

        MultiControlInputManager* multiControlInputManager =
            reinterpret_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(
            MenuInputManager::InputType::Option, nullptr
        );

        delete s_onOption;
        s_onOption = nullptr;
    }

    void onRefocus() override
    {
        LONGCALL void Page_onRefocus(Page * page)
            AT(RMCXD_PORT(0x805BB228, 0x805B5668, 0x805BABA8, 0x805A9280));

        Page_onRefocus(this);

        transition(resolve());
    }

private:
    enum class State {
        Previous,
        Prompt,
    };

    State resolve()
    {
        switch (s_state) {
        case State::Previous:
            break;
        case State::Prompt:
            return State::Previous;
        }

        return s_state;
    }

    void transition(State state)
    {
        Section* section = SectionManager::Instance()->currentSection();
        YesNoPopupPage* yesNoPopupPage =
            section->page<YesNoPopupPage>(PageId::YesNoPopup);

        if (state == s_state) {
            return;
        }

        switch (state) {
        case State::Previous: {
            break;
        }
        case State::Prompt: {
            FormatParam formatParam{};
            formatParam.strings[0] =
                L"Enable Open Host?\n\n"
                L"This features allows players who\n"
                L"add your friend code to meet up with you,\n"
                L"even if you don't add them back.";

            yesNoPopupPage->reset();
            yesNoPopupPage->setWindowMessage(0x19CA, &formatParam);
            yesNoPopupPage->configureButton(
                0, 0xFAC, nullptr, Animation::None, nullptr
            );
            yesNoPopupPage->configureButton(
                1, 0xFAD, nullptr, Animation::None, nullptr
            );
            yesNoPopupPage->setDefaultChoice(1);

            push(PageId::YesNoPopup, Animation::Next);
            break;
        }
        }

        s_state = state;
    }

    void onOption(u32 /* localPlayerId */)
    {
        transition(State::Prompt);
    }

    /* 0x044 */ u8 _044[0xF34 - 0x044];

    static State s_state;
    static mkw::UI::MenuInputManager::Handler<WifiFriendMenuPage>* s_onOption;
};

static_assert(sizeof(WifiFriendMenuPage) == 0xF34);

#endif

} // namespace mkw::UI

#if RMC

extern "C" {
static void
WifiFriendMenu_onActivate(mkw::UI::WifiFriendMenuPage* wifiFriendMenuPage)
{
    wifiFriendMenuPage->WifiFriendMenuPage::onActivate();
}

static void
WifiFriendMenu_onDeactivate(mkw::UI::WifiFriendMenuPage* wifiFriendMenuPage)
{
    wifiFriendMenuPage->WifiFriendMenuPage::onDeactivate();
}

static void
WifiFriendMenu_onRefocus(mkw::UI::WifiFriendMenuPage* wifiFriendMenuPage)
{
    wifiFriendMenuPage->WifiFriendMenuPage::onRefocus();
}
}

#endif
