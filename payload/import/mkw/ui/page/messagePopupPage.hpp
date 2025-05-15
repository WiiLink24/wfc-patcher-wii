#pragma once

#if RMC

#  include "messagePage.hpp"

namespace wwfc::mkw::UI
{

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

} // namespace wwfc::mkw::UI

#endif // RMC