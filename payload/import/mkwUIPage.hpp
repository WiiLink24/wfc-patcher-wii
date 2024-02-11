#pragma once

#include "mkwUISectionManager.hpp"

namespace mkw::UI
{

#if RMC

class Page
{
public:
    enum class Animation {
        None = -1,
        Next = 0,
        Previous = 1,
    };

    virtual ~Page();
    virtual void dt(s32 type);
    virtual void vf_0C();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1C();
    virtual void vf_20();
    virtual void push(PageId pageId, Animation animation);
    virtual void vf_28();
    virtual void vf_2C();
    virtual void vf_30();
    virtual void vf_34();
    virtual void vf_38();
    virtual void vf_3C();
    virtual void vf_40();
    virtual void vf_44();
    virtual void vf_48();
    virtual void vf_4C();
    virtual void vf_50();
    virtual void vf_54();
    virtual void vf_58();
    virtual void vf_5C();
    virtual void vf_60();

private:
    /* 0x04 */ u8 _04[0x44 - 0x04];
};

static_assert(sizeof(Page) == 0x44);

class MessagePage : public Page
{
public:
    virtual void reset();
    virtual void
    setWindowMessage(u32 messageId, FormatParam* formatParam = nullptr) = 0;
    virtual void vf_6C() = 0;

private:
    /* 0x44 */ u8 _044[0x1A8 - 0x044];
};

static_assert(sizeof(MessagePage) == 0x1A8);

class MessagePopupPage : public MessagePage
{
public:
    void reset() override;
    void setWindowMessage(u32 messageId, FormatParam* formatParam = nullptr)
        override;

private:
    /* 0x1A8 */ u8 _1A8[0x604 - 0x1A8];
};

static_assert(sizeof(MessagePopupPage) == 0x604);

class WifiMenuPage : public Page
{
public:
    void showMessageOfTheDay()
    {
        Section* section = SectionManager::Instance()->currentSection();
        MessagePopupPage* messagePopupPage =
            section->page<MessagePopupPage>(PageId::MessagePopup);

        FormatParam formatParam{};
        formatParam.strings[0] = L"Welcome to\nWiiLink Wi-Fi Connection!";

        messagePopupPage->reset();
        messagePopupPage->setWindowMessage(0x19CA, &formatParam);

        push(PageId::MessagePopup, Animation::Next);
    }

    static bool HasSeenMessageOfTheDay()
    {
        return s_hasSeenMessageOfTheDay;
    }

    static void SeenMessageOfTheDay()
    {
        s_hasSeenMessageOfTheDay = true;
    }

private:
    /* 0x044 */ u8 _044[0xF34 - 0x044];

    static bool s_hasSeenMessageOfTheDay;
};

static_assert(sizeof(WifiMenuPage) == 0xF34);

#endif

} // namespace mkw::UI
