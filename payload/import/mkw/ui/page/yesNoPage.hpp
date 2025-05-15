#pragma once

#if RMC

#  include "import/mkw/ui/ui.hpp"
#  include "page.hpp"
#  include <wwfcUtil.h>

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/YesNoPage.hh
class YesNoPage : public Page
{
public:
    class IHandler
    {
    private:
        virtual void dummy_00()
        {
        }

        virtual void dummy_04()
        {
        }

    public:
        virtual void handle(int choice, void* pushButton) = 0;
    };

    template <typename T>
    class Handler final : public IHandler
    {
    public:
        Handler(T* object, void (T::*function)(int choice, void* pushButton))
        {
            m_object = object;
            m_function = function;
        }

        void handle(int choice, void* pushButton) override
        {
            (m_object->*m_function)(choice, pushButton);
        }

    private:
        T* m_object;
        void (T::*m_function)(int choice, void* pushButton);
    };

    virtual void reset();

    void setWindowMessage(u32 messageId, FormatParam* formatParam = nullptr)
    {
        LONGCALL void setWindowMessage(
            YesNoPage * yesNoPage, u32 messageId,
            FormatParam* formatParam = nullptr
        ) AT(RMCXD_PORT(0x806525FC, 0x8061EBE8, 0x80651C68, 0x80640914));

        setWindowMessage(this, messageId, formatParam);
    }

    void configureButton(
        u32 index, u32 messageId, FormatParam* formatParam, Animation animation,
        IHandler* handler
    )
    {
        LONGCALL void configureButton(
            YesNoPage * yesNoPage, u32 index, u32 messageId,
            FormatParam * formatParam, Animation animation, IHandler * handler
        ) AT(RMCXD_PORT(0x80652604, 0x8061EBF0, 0x80651C70, 0x8064091C));

        configureButton(
            this, index, messageId, formatParam, animation, handler
        );
    }

    void setDefaultChoice(u32 defaultChoice)
    {
        m_defaultChoice = defaultChoice;
    }

private:
    /* 0x044 */ u8 _044[0x27C - 0x044];
    /* 0x27C */ u32 m_defaultChoice;
    /* 0x280 */ u8 _280[0x8B8 - 0x280];
};

static_assert(sizeof(YesNoPage) == 0x8B8);

} // namespace wwfc::mkw::UI

#endif // RMC
