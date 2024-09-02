#pragma once

#include "import/cxx.hpp"
#include "import/dwc.h"
#include "import/mkw/system/system.hpp"
#include "import/mkw/ui/multiMenuInputManager.hpp"
#include "import/mkw/ui/section/sectionManager.hpp"
#include "import/revolution.h"
#include "messagePopupPage.hpp"
#include "yesNoPopupPage.hpp"

namespace mkw::UI
{

#if RMC

class OpenHostPage : public Page
{
public:
    void onActivate() override
    {
        EGG::Heap* systemHeap = mkw::System::System::Instance().systemHeap();

        s_onOption =
            new (systemHeap, 4) MenuInputManager::Handler<OpenHostPage>(
                this, &OpenHostPage::onOption
            );
        MultiControlInputManager* multiControlInputManager =
            reinterpret_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(
            MenuInputManager::InputType::Option, s_onOption
        );

        s_onYesOrNo = new (systemHeap, 4)
            YesNoPage::Handler<OpenHostPage>(this, &OpenHostPage::onYesOrNo);
    }

    void onDeactivate() override
    {
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
        transition(resolve());
    }

private:
    OpenHostPage();

    enum class State {
        Previous,
        Prompt,
        Result,
    };

    const wchar_t* openHostPromptMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostPromptMessage =
            s_openHostPromptMessages[language];
        if (openHostPromptMessage) {
            return openHostPromptMessage;
        }

        return s_openHostPromptMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* connectionLostMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* connectionLostMessage =
            s_connectionLostMessages[language];
        if (connectionLostMessage) {
            return connectionLostMessage;
        }

        return s_connectionLostMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* openHostEnabledMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostEnabledMessage =
            s_openHostEnabledMessages[language];
        if (openHostEnabledMessage) {
            return openHostEnabledMessage;
        }

        return s_openHostEnabledMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* openHostDisabledMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostDisabledMessage =
            s_openHostDisabledMessages[language];
        if (openHostDisabledMessage) {
            return openHostDisabledMessage;
        }

        return s_openHostDisabledMessages[RVL::SCLanguageEnglish];
    }

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
            formatParam.strings[0] = openHostPromptMessage();

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
                formatParam.strings[0] = connectionLostMessage();
            } else {
                if (s_openHostEnabled) {
                    formatParam.strings[0] = openHostEnabledMessage();
                } else {
                    formatParam.strings[0] = openHostDisabledMessage();
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

        bool openHostEnabled = choice == 0;

        char openHostValue[2];
        openHostValue[0] = '0' + openHostEnabled;
        openHostValue[1] = '\0';
        GameSpy::gpiSendLocalInfo(
            gpConnection, "\\wwfc_openhost\\", openHostValue
        );

        s_openHostEnabled = openHostEnabled;
    }

    static State s_state;
    static mkw::UI::MenuInputManager::Handler<OpenHostPage>* s_onOption;
    static mkw::UI::YesNoPage::Handler<OpenHostPage>* s_onYesOrNo;
    static bool s_openHostEnabled;
    static bool s_sentOpenHostValue;
    static const wchar_t* s_openHostPromptMessages[RVL::SCLanguageCount];
    static const wchar_t* s_connectionLostMessages[RVL::SCLanguageCount];
    static const wchar_t* s_openHostEnabledMessages[RVL::SCLanguageCount];
    static const wchar_t* s_openHostDisabledMessages[RVL::SCLanguageCount];
};

static_assert(sizeof(OpenHostPage) == sizeof(Page));

#endif

} // namespace mkw::UI
