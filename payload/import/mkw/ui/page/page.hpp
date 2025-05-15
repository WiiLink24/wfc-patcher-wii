#pragma once

#if RMC

#  include "import/mkw/ui/menuInputManager.hpp"
#  include "pageId.hpp"

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/Page.hh
class Page
{
public:
    enum class Animation {
        None = -1,
        Next = 0,
        Previous = 1,
    };

    Page();
    virtual ~Page();
    virtual void dt(s32 type);
    virtual void vf_0C();
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1C();
    virtual void vf_20();
    virtual void push(PageId pageId, Animation animation);
    virtual void vf_28();
    virtual void vf_2C();
    virtual void onActivate();
    virtual void onDeactivate();
    virtual void vf_38();
    virtual void vf_3C();
    virtual void vf_40();
    virtual void vf_44();
    virtual void vf_48();
    virtual void vf_4C();
    virtual void vf_50();
    virtual void onRefocus();
    virtual void vf_58();
    virtual void vf_5C();
    virtual void vf_60();

    MenuInputManager* menuInputManager()
    {
        return m_menuInputManager;
    }

private:
    /* 0x04 */ u8 _04[0x38 - 0x04];
    /* 0x38 */ MenuInputManager* m_menuInputManager;
    /* 0x3C */ u8 _3C[0x44 - 0x3C];
};

static_assert(sizeof(Page) == 0x44);

} // namespace wwfc::mkw::UI

#endif // RMC