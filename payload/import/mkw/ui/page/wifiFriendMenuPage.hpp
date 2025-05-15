#pragma once

#if RMC

#  include "openHostPage.hpp"

namespace wwfc::mkw::UI
{

class WifiFriendMenuPage : public OpenHostPage
{
public:
    void onActivate() override
    {
        LONGCALL void onActivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CF18, 0x80619C04, 0x8064C584, 0x8063B230));

        onActivate(this);

        OpenHostPage::onActivate();
    }

    void onDeactivate() override
    {
        LONGCALL void onDeactivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CFF8, 0x80619CE4, 0x8064C664, 0x8063B310));

        onDeactivate(this);

        OpenHostPage::onDeactivate();
    }

    void onRefocus() override
    {
        LONGCALL void Page_onRefocus(Page * page)
            AT(RMCXD_PORT(0x805BB228, 0x805B5668, 0x805BABA8, 0x805A9280));

        Page_onRefocus(this);

        OpenHostPage::onRefocus();
    }

private:
    /* 0x044 */ u8 _044[0xF34 - 0x044];
};

static_assert(sizeof(WifiFriendMenuPage) == 0xF34);

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

} // namespace wwfc::mkw::UI

#endif // RMC