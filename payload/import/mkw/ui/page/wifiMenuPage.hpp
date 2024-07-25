#pragma once

#include "import/mkw/ui/section/sectionManager.hpp"
#include "messagePopupPage.hpp"
#include <cstddef>

namespace mkw::UI
{

#if RMC

class WifiMenuPage : public Page
{
public:
    void showMessage()
    {
        if (!s_message) {
            return;
        }

        pushMessagePopup(s_message);
        s_message = nullptr;
    }

    static void SetMessage(const wchar_t* message)
    {
        s_message = message;
    }

    static bool HasSeenMessageOfTheDay()
    {
        return s_hasSeenMessageOfTheDay;
    }

    static void SeenMessageOfTheDay()
    {
        s_hasSeenMessageOfTheDay = true;
    }

    static wchar_t* MessageOfTheDayBuffer()
    {
        return s_messageOfTheDayBuffer;
    }

    static size_t MessageOfTheDayBufferSize()
    {
        return sizeof(s_messageOfTheDayBuffer);
    }

private:
    void pushMessagePopup(const wchar_t* message)
    {
        Section* section = SectionManager::Instance()->currentSection();
        MessagePopupPage* messagePopupPage =
            section->page<MessagePopupPage>(PageId::MessagePopup);

        FormatParam formatParam{};
        formatParam.strings[0] = message;

        messagePopupPage->reset();
        messagePopupPage->setWindowMessage(0x19CA, &formatParam);

        push(PageId::MessagePopup, Animation::Next);
    }

    /* 0x044 */ u8 _044[0xF34 - 0x044];

    static const wchar_t* s_message;
    static bool s_hasSeenMessageOfTheDay;
    static wchar_t s_messageOfTheDayBuffer[256];
};

static_assert(sizeof(WifiMenuPage) == 0xF34);

#endif

} // namespace mkw::UI

#if RMC

extern "C" {
__attribute__((__used__)) static void
WifiMenuPage_showMessage(mkw::UI::WifiMenuPage* wifiMenuPage)
{
    wifiMenuPage->showMessage();
}
}

#endif
