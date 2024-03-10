#pragma once

#include "messagePage.hpp"

namespace mkw::UI
{

#if RMC

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/MessagePage.hh
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

#endif

} // namespace mkw::UI
