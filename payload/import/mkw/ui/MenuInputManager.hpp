#pragma once

#if RMC

#  include <wwfcTypes.h>

namespace wwfc::mkw::UI
{

// https://github.com/mkw-sp/mkw-sp/blob/main/payload/game/ui/MenuInputManager.hh
class MenuInputManager
{
public:
    enum class InputType {
        Option = 2,
    };

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
        virtual void handle(u32 localPlayerId) = 0;
    };

    template <typename T>
    class Handler final : public IHandler
    {
    public:
        Handler(T* object, void (T::*function)(u32 localPlayerId))
        {
            m_object = object;
            m_function = function;
        }

        void handle(u32 localPlayerId) override
        {
            (m_object->*m_function)(localPlayerId);
        }

    private:
        T* m_object;
        void (T::*m_function)(u32 localPlayerId);
    };

    MenuInputManager();
    virtual ~MenuInputManager();

private:
    /* 0x04 */ u8 _04[0x10 - 0x04];
};

static_assert(sizeof(MenuInputManager) == 0x10);

} // namespace wwfc::mkw::UI

#endif // RMC
