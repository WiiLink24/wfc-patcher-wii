#pragma once

#if RMC

#  include "import/mkw/net/net.hpp"
#  include "openHostPage.hpp"

namespace wwfc::mkw::UI
{

class FriendRoomPage : public OpenHostPage
{
public:
    void onActivate() override
    {
        LONGCALL void onActivate(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D8444, 0x806289B8, 0x805D7D20, 0x805C65E0));

        onActivate(this);

        if (mkw::Net::NetController::Instance()->amITheRoomHost()) {
            OpenHostPage::onActivate();
        }
    }

    void onDeactivate() override
    {
        LONGCALL void onDeactivate(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D84FC, 0x80628A70, 0x805D7DD8, 0x805C6698));

        onDeactivate(this);

        if (mkw::Net::NetController::Instance()->amITheRoomHost()) {
            OpenHostPage::onDeactivate();
        }
    }

    void onRefocus() override
    {
        LONGCALL void onRefocus(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D8C98, 0x8062920C, 0x805D8574, 0x805C6E34));

        onRefocus(this);

        if (mkw::Net::NetController::Instance()->amITheRoomHost()) {
            OpenHostPage::onRefocus();
        }
    }

private:
    /* 0x044 */ u8 _044[0xDC4 - 0x044];
};

static_assert(sizeof(FriendRoomPage) == 0xDC4);

extern "C" {
static void FriendRoomPage_onActivate(mkw::UI::FriendRoomPage* friendRoomPage)
{
    friendRoomPage->FriendRoomPage::onActivate();
}

static void FriendRoomPage_onDeactivate(mkw::UI::FriendRoomPage* friendRoomPage)
{
    friendRoomPage->FriendRoomPage::onDeactivate();
}

static void FriendRoomPage_onRefocus(mkw::UI::FriendRoomPage* friendRoomPage)
{
    friendRoomPage->FriendRoomPage::onRefocus();
}
} // extern "C"

} // namespace wwfc::mkw::UI

#endif // RMC