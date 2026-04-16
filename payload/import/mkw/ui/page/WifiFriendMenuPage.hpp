#pragma once

#if RMC

#  include "Page.hpp"

namespace wwfc::mkw::UI
{

class WifiFriendMenuPage : public Page
{
public:
    void onActivate() override
    {
        [[gnu::longcall]] void onActivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CF18, 0x80619C04, 0x8064C584, 0x8063B230, 0x8064D44C));

        return onActivate(this);
    }

    void onDeactivate() override
    {
        [[gnu::longcall]] void onDeactivate(WifiFriendMenuPage * wifiFriendMenuPage)
            AT(RMCXD_PORT(0x8064CFF8, 0x80619CE4, 0x8064C664, 0x8063B310, 0x8064D52C));

        return onDeactivate(this);
    }

    void onRefocus() override
    {
        [[gnu::longcall]] void Page_onRefocus(Page * page)
            AT(RMCXD_PORT(0x805BB228, 0x805B5668, 0x805BABA8, 0x805A9280, 0x805B9C8C));

        return Page_onRefocus(this);
    }

private:
    /* 0x044 */ u8 _044[0xF34 - 0x044];
};

static_assert(sizeof(WifiFriendMenuPage) == 0xF34);

} // namespace wwfc::mkw::UI

#endif // RMC