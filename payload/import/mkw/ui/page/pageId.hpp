#pragma once

namespace mkw::UI
{

#if RMC

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/PageId.hh
enum class PageId {
    MessagePopup = 0x4D,
    YesNoPopup = 0x4E,
    WifiFriendMenu = 0x8D,
};

#endif

} // namespace mkw::UI
