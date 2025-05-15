#pragma once

#if RMC

#  include "menuInputManager.hpp"
#  include <wwfcUtil.h>

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/MenuInputManager.hh
class MultiControlInputManager : public MenuInputManager
{
public:
    void setHandler(
        InputType inputType, IHandler* handler, bool register6 = false,
        bool register7 = false
    )
    {
        LONGCALL void setHandler(
            MultiControlInputManager * multiControlInputManager,
            InputType inputType, IHandler * handler, bool register6 = false,
            bool register7 = false
        ) AT(RMCXD_PORT(0x805F0D84, 0x805D62B0, 0x805F0660, 0x805DF1A4));

        setHandler(this, inputType, handler, register6, register7);
    }

private:
    /* 0x010 */ u8 _010[0x224 - 0x010];
};

static_assert(sizeof(MultiControlInputManager) == 0x224);

} // namespace wwfc::mkw::UI

#endif // RMC