#pragma once

#include "import/cxx.hpp"
#include "import/dwc.h"
#include "import/gamespy.h"
#include "import/mkw/system/system.hpp"
#include "messagePopupPage.hpp"
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

        EGG::Heap* systemHeap = mkw::System::System::Instance().systemHeap();

        s_onOption =
            new (systemHeap, 4) MenuInputManager::Handler<WifiFriendMenuPage>(
                this, &WifiFriendMenuPage::onOption
            );
        MultiControlInputManager* multiControlInputManager =
            reinterpret_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(
            MenuInputManager::InputType::Option, s_onOption
        );

        s_onYesOrNo =
            new (systemHeap, 4) YesNoPage::Handler<WifiFriendMenuPage>(
                this, &WifiFriendMenuPage::onYesOrNo
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

        delete s_onYesOrNo;
        s_onYesOrNo = nullptr;
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
        Result,
    };

    State resolve() const
    {
        switch (s_state) {
        case State::Previous: {
            break;
        }
        case State::Prompt: {
            return State::Result;
        }
        case State::Result: {
            return State::Previous;
        }
        }

        return s_state;
    }

    void transition(State state)
    {
        Section* section = SectionManager::Instance()->currentSection();

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
                L"This feature allows players who\n"
                L"add your friend code to meet up with you,\n"
                L"even if you don't add them back.";

            YesNoPopupPage* yesNoPopupPage =
                section->page<YesNoPopupPage>(PageId::YesNoPopup);
            yesNoPopupPage->reset();
            yesNoPopupPage->setWindowMessage(0x19CA, &formatParam);
            yesNoPopupPage->configureButton(
                0, 0xFAC, nullptr, Animation::None, s_onYesOrNo
            );
            yesNoPopupPage->configureButton(
                1, 0xFAD, nullptr, Animation::None, s_onYesOrNo
            );
            yesNoPopupPage->setDefaultChoice(1);

            push(PageId::YesNoPopup, Animation::Next);
            break;
        }
        case State::Result: {
            FormatParam formatParam{};
            if (!s_sentOpenHostValue) {
                formatParam.strings[0] = L"You have lost connection to\n"
                                         L"the server.\n\n"
                                         L"Please try again later.";
            } else {
                if (s_enableOpenHost) {
                    formatParam.strings[0] = L"Open Host is now enabled!";
                } else {
                    formatParam.strings[0] = L"Open Host is now disabled!";
                }
            }

            MessagePopupPage* messagePopupPage =
                section->page<MessagePopupPage>(PageId::MessagePopup);
            messagePopupPage->reset();
            messagePopupPage->setWindowMessage(0x19CA, &formatParam);

            push(PageId::MessagePopup, Animation::Next);
            break;
        }
        }

        s_state = state;
    }

    void onOption(u32 /* localPlayerId */)
    {
        transition(State::Prompt);
    }

    void onYesOrNo(int choice, void* /* pushButton */)
    {
        GameSpy::GPConnection* gpConnection = DWC::stpMatchCnt->connection;
        if (!gpConnection) {
            s_sentOpenHostValue = false;
            return;
        }
        s_sentOpenHostValue = true;

        bool enableOpenHost = choice == 0;

        char openHostValue[2];
        openHostValue[0] = '0' + enableOpenHost;
        openHostValue[1] = '\0';
        GameSpy::gpiSendLocalInfo(
            gpConnection, "\\wwfc_openhost\\", openHostValue
        );

        s_enableOpenHost = enableOpenHost;
    }

    /* 0x044 */ u8 _044[0xF34 - 0x044];

    static State s_state;
    static mkw::UI::MenuInputManager::Handler<WifiFriendMenuPage>* s_onOption;
    static mkw::UI::YesNoPage::Handler<WifiFriendMenuPage>* s_onYesOrNo;
    static bool s_enableOpenHost;
    static bool s_sentOpenHostValue;
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
