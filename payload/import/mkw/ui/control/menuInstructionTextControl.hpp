#pragma once

#include "import/mkw/ui/ui.hpp"
#include "uiControl.hpp"

namespace mkw::UI
{

#if RMC

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/ctrl/CtrlMenuInstructionText.hh
class MenuInstructionTextControl : public LayoutUIControl
{
public:
    MenuInstructionTextControl();
    ~MenuInstructionTextControl() override;

    void setMessage(s32 messageId, FormatParam* formatParam = nullptr)
    {
        LONGCALL void setMessage(
            MenuInstructionTextControl * menuInstructionTextControl,
            s32 messageId, FormatParam* formatParam = nullptr
        ) AT(RMCXD_PORT(0x807E9A38, 0x80841960, 0x807E90A4, 0x807D7DF8));

        setMessage(this, messageId, formatParam);
    }
};

static_assert(sizeof(MenuInstructionTextControl) == 0x174);

#endif

} // namespace mkw::UI
