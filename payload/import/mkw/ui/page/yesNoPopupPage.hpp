#pragma once

#if RMC

#  include "yesNoPage.hpp"

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/YesNoPage.hh
class YesNoPopupPage : public YesNoPage
{
private:
    /* 0x8B8 */ u8 _8B8[0xBA0 - 0x8B8];
};

static_assert(sizeof(YesNoPopupPage) == 0xBA0);

} // namespace wwfc::mkw::UI

#endif // RMC
