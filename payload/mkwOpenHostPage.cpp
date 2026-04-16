#pragma once

#if RMC

#  include "import/dwc.h"
#  include "import/egg/core/eggHeap.hpp"
#  include "import/mkw/system/System.hpp"
#  include "import/mkw/ui/MultiMenuInputManager.hpp"
#  include "import/mkw/ui/page/FriendRoomPage.hpp"
#  include "import/mkw/ui/page/MessagePopupPage.hpp"
#  include "import/mkw/ui/page/YesNoPopupPage.hpp"
#  include "import/mkw/ui/page/WifiFriendMenuPage.hpp"
#  include "import/mkw/ui/section/SectionManager.hpp"
#  include "import/revolution.h"
#  include "wwfcPatch.hpp"

namespace wwfc::mkw::UI
{

class OpenHostPage : public Page
{
public:
    void onActivate() override
    {
        EGG::Heap* systemHeap = mkw::System::System::Instance().getSystemHeap();

        s_onOption = new (systemHeap, 4)
            MenuInputManager::Handler<OpenHostPage>(this, &OpenHostPage::onOption);
        MultiControlInputManager* multiControlInputManager =
            static_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(MenuInputManager::InputType::Option, s_onOption);

        s_onYesOrNo =
            new (systemHeap, 4) YesNoPage::Handler<OpenHostPage>(this, &OpenHostPage::onYesOrNo);
    }

    void onDeactivate() override
    {
        MultiControlInputManager* multiControlInputManager =
            static_cast<MultiControlInputManager*>(menuInputManager());
        multiControlInputManager->setHandler(MenuInputManager::InputType::Option, nullptr);
        EGG::Heap::Free(s_onOption, nullptr);
        s_onOption = nullptr;

        EGG::Heap::Free(s_onYesOrNo, nullptr);
        s_onYesOrNo = nullptr;
    }

    void onRefocus() override
    {
        transition(resolve());
    }

private:
    OpenHostPage();

    enum class EState {
        PREVIOUS,
        PROMPT,
        RESULT,
    };

    const wchar_t* openHostPromptMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostPromptMessage = s_openHostPromptMessages[language];
        if (openHostPromptMessage) {
            return openHostPromptMessage;
        }

        return s_openHostPromptMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* connectionLostMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* connectionLostMessage = s_connectionLostMessages[language];
        if (connectionLostMessage) {
            return connectionLostMessage;
        }

        return s_connectionLostMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* openHostEnabledMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostEnabledMessage = s_openHostEnabledMessages[language];
        if (openHostEnabledMessage) {
            return openHostEnabledMessage;
        }

        return s_openHostEnabledMessages[RVL::SCLanguageEnglish];
    }

    const wchar_t* openHostDisabledMessage() const
    {
        u8 language = RVL::SCGetLanguage();

        const wchar_t* openHostDisabledMessage = s_openHostDisabledMessages[language];
        if (openHostDisabledMessage) {
            return openHostDisabledMessage;
        }

        return s_openHostDisabledMessages[RVL::SCLanguageEnglish];
    }

    EState resolve() const
    {
        switch (s_state) {
        case EState::PREVIOUS:
            break;
        case EState::PROMPT:
            return EState::RESULT;
        case EState::RESULT:
            return EState::PREVIOUS;
        }
        return s_state;
    }

    void transition(EState state)
    {
        Section* section = SectionManager::Instance()->currentSection();

        if (state == s_state) {
            return;
        }

        switch (state) {
        case EState::PREVIOUS:
            break;

        case EState::PROMPT: {
            FormatParam formatParam{};
            formatParam.strings[0] = openHostPromptMessage();

            YesNoPopupPage* yesNoPopupPage =
                section->getPage<YesNoPopupPage>(EPageID::POPUP_YES_NO);
            yesNoPopupPage->reset();
            yesNoPopupPage->setWindowMessage(0x19CA, &formatParam);
            yesNoPopupPage->configureButton(0, 0xFAC, nullptr, ENextType::NONE, s_onYesOrNo);
            yesNoPopupPage->configureButton(1, 0xFAD, nullptr, ENextType::NONE, s_onYesOrNo);
            yesNoPopupPage->setDefaultChoice(1);

            push(EPageID::POPUP_YES_NO, ENextType::FORWARD);
            break;
        }

        case EState::RESULT:
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
                section->getPage<MessagePopupPage>(EPageID::POPUP_MESSAGE);
            messagePopupPage->reset();
            messagePopupPage->setWindowMessage(0x19CA, &formatParam);

            push(EPageID::POPUP_MESSAGE, ENextType::FORWARD);
            break;
        }

        s_state = state;
    }

    void onOption(u32 /* localPlayerId */)
    {
        transition(EState::PROMPT);
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
        GameSpy::gpiSendLocalInfo(gpConnection, "\\wl:oh\\", openHostValue);

        s_openHostEnabled = openHostEnabled;
    }

    static EState                                            s_state;
    static mkw::UI::MenuInputManager::Handler<OpenHostPage>* s_onOption;
    static mkw::UI::YesNoPage::Handler<OpenHostPage>*        s_onYesOrNo;
    static bool                                              s_openHostEnabled, s_sentOpenHostValue;
    static const wchar_t* s_openHostPromptMessages[RVL::SCLanguageCount];
    static const wchar_t* s_connectionLostMessages[RVL::SCLanguageCount];
    static const wchar_t* s_openHostEnabledMessages[RVL::SCLanguageCount];
    static const wchar_t* s_openHostDisabledMessages[RVL::SCLanguageCount];
};

static_assert(sizeof(OpenHostPage) == sizeof(Page));

OpenHostPage::EState                     OpenHostPage::s_state     = OpenHostPage::EState::PREVIOUS;
MenuInputManager::Handler<OpenHostPage>* OpenHostPage::s_onOption  = nullptr;
YesNoPage::Handler<OpenHostPage>*        OpenHostPage::s_onYesOrNo = nullptr;
bool                                     OpenHostPage::s_openHostEnabled    = false;
bool                                     OpenHostPage::s_sentOpenHostValue  = false;
const wchar_t* OpenHostPage::s_openHostPromptMessages[RVL::SCLanguageCount] = {
    L"こうかいホストをゆうこうにしますか？\n"
    L"\n"
    L"この機能はあなたのフレンドコードを\n"
    L"追加したプレイヤーはあなたが追加し返さなくても\n"
    L"フレンドとしてあなたと会えるようになる機能です",
    L"Enable Open Host?\n"
    L"\n"
    L"This feature allows players who\n"
    L"add your friend code to meet up with you,\n"
    L"even if you don't add them back.",
    nullptr,
    L"Activer l'Open Host?\n"
    L"\n"
    L"Cette fonctionnalité permet aux joueurs qui\n"
    L"ajoutent votre code ami de vous rejoindre,\n"
    L"même si vous ne les avez pas ajoutés.",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
const wchar_t* OpenHostPage::s_connectionLostMessages[RVL::SCLanguageCount] = {
    L"サーバーからの接続が切断されました\n"
    L"\n"
    L"もう一度やり直してください",
    L"You have lost connection to\n"
    L"the server.\n"
    L"\n"
    L"Please try again later.",
    nullptr,
    L"Vous avez perdu la connexion\n"
    L"au serveur.\n"
    L"\n"
    L"Veuillez réessayer ultérieurement.",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
const wchar_t* OpenHostPage::s_openHostEnabledMessages[RVL::SCLanguageCount] = {
    L"こうかいホストをゆうこうにしました！",
    L"Open Host is now enabled!",
    nullptr,
    L"Open Host est maintenant activé!",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};
const wchar_t* OpenHostPage::s_openHostDisabledMessages[RVL::SCLanguageCount] = {
    L"こうかいホストをむこうにしました！",
    L"Open Host is now disabled!",
    nullptr,
    L"Open Host est maintenant désactivé!",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

// Allow users to open rooms without having any friends added
WWFC_DEFINE_PATCH = 
    Patch::CallWithCTR( //
        WWFC_PATCH_LEVEL_FEATURE, //
        RMCXD_PORT(0x8064D358, 0x8061A044, 0x8064C9C4, 0x8063B670, 0x8064D88C), //
        [](mkw::UI::WifiFriendMenuPage* /* wifiFriendMenuPage */,
           void* /* pushButton */) -> int {
    constexpr int friendsAdded = 1;

    return friendsAdded;
}
    );

// Allow the "Open Host" feature to be enabled via the press of a button
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808B9008, 0x808BABF8, 0x808B8158, 0x808A7470, 0x808B9760), //

    [](mkw::UI::Page* page) {
        static_cast<FriendRoomPage*>(page)->FriendRoomPage::onActivate();

        if (mkw::NetManager::Instance()->amITheRoomHost()) {
            static_cast<OpenHostPage*>(page)->OpenHostPage::onActivate();
        }
    }
);
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808B900C, 0x808BABFC, 0x808B815C, 0x808A7474, 0x808B9764), //

    [](mkw::UI::Page* page) {
        static_cast<FriendRoomPage*>(page)->FriendRoomPage::onDeactivate();

        if (mkw::NetManager::Instance()->amITheRoomHost()) {
            static_cast<OpenHostPage*>(page)->OpenHostPage::onDeactivate();
        }
    }
);
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808B902C, 0x808BAC1C, 0x808B817C, 0x808A7494, 0x808B9784), //

    [](mkw::UI::Page* page) {
        static_cast<FriendRoomPage*>(page)->FriendRoomPage::onRefocus();

        if (mkw::NetManager::Instance()->amITheRoomHost()) {
            static_cast<OpenHostPage*>(page)->OpenHostPage::onRefocus();
        }
    }
);
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808BFE7C, 0x808B97CC, 0x808BEFCC, 0x808AE2EC, 0x808C08BC), //

    [](mkw::UI::Page* page) {
        static_cast<WifiFriendMenuPage*>(page)->WifiFriendMenuPage::onActivate();
        static_cast<OpenHostPage*>(page)->OpenHostPage::onActivate();
    }
);
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808BFE80, 0x808B97D0, 0x808BEFD0, 0x808AE2F0, 0x808C08C0), //

    [](mkw::UI::Page* page) {
        static_cast<WifiFriendMenuPage*>(page)->WifiFriendMenuPage::onDeactivate();
        static_cast<OpenHostPage*>(page)->OpenHostPage::onDeactivate();
    }
);
WWFC_DEFINE_PATCH = Patch::WritePointer(
    WWFC_PATCH_LEVEL_FEATURE,
    RMCXD_PORT(0x808BFEA0, 0x808B97F0, 0x808BEFF0, 0x808AE310, 0x808C08E0), //

    [](mkw::UI::Page* page) {
        static_cast<WifiFriendMenuPage*>(page)->WifiFriendMenuPage::onRefocus();
        static_cast<OpenHostPage*>(page)->OpenHostPage::onRefocus();
    }
);

} // namespace wwfc::mkw::UI

#endif // RMC