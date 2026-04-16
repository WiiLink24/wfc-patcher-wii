#pragma once

#if RMC

#  include "Page.hpp"
#  include "import/mkw/net/NetManager.hpp"

namespace wwfc::mkw::UI
{

class FriendRoomPage : public Page
{
public:
    void onActivate() override
    {
        [[gnu::longcall]] void onActivate(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D8444, 0x806289B8, 0x805D7D20, 0x805C65E0, 0x805D6ECC));

        return onActivate(this);
    }

    void onDeactivate() override
    {
        [[gnu::longcall]] void onDeactivate(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D84FC, 0x80628A70, 0x805D7DD8, 0x805C6698, 0x805D6F84));

        return onDeactivate(this);
    }

    void onRefocus() override
    {
        [[gnu::longcall]] void onRefocus(FriendRoomPage * friendRoomPage)
            AT(RMCXD_PORT(0x805D8C98, 0x8062920C, 0x805D8574, 0x805C6E34, 0x805D7720));

        return onRefocus(this);
    }

private:
    /* 0x044 */ u8 _044[0xDC4 - 0x044];
};

static_assert(sizeof(FriendRoomPage) == 0xDC4);

} // namespace wwfc::mkw::UI

#endif // RMC