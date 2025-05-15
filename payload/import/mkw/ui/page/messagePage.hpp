#pragma once

#if RMC

#  include "import/mkw/ui/ui.hpp"
#  include "page.hpp"

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/MessagePage.hh
class MessagePage : public Page
{
public:
    virtual void reset();
    virtual void
    setWindowMessage(u32 messageId, FormatParam* formatParam = nullptr) = 0;
    virtual void vf_6C() = 0;

private:
    /* 0x044 */ u8 _044[0x1A8 - 0x044];
};

static_assert(sizeof(MessagePage) == 0x1A8);

} // namespace wwfc::mkw::UI

#endif // RMC