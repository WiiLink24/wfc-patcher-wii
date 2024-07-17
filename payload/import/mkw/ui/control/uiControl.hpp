#pragma once

#include <wwfcCommon.h>

namespace mkw::UI
{

#if RMC

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/UIControl.hh
class UIControl
{
public:
    UIControl();
    virtual ~UIControl();
    virtual void dt(s32 type);
    virtual void vf_08();
    virtual void vf_0C();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1C();
    virtual void vf_20();
    virtual void vf_24();
    virtual void vf_28();
    virtual void vf_2C();
    virtual void vf_30();
    virtual void vf_34();

private:
    /* 0x04 */ u8 _04[0x98 - 0x04];
};

static_assert(sizeof(UIControl) == 0x98);

class LayoutUIControl : public UIControl
{
public:
    LayoutUIControl();
    ~LayoutUIControl() override;
    virtual void vf_38();

private:
    /* 0x098 */ u8 _098[0x174 - 0x098];
};

static_assert(sizeof(LayoutUIControl) == 0x174);

#endif

} // namespace mkw::UI
