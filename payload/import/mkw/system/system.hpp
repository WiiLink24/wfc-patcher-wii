#pragma once

#include "import/egg/sceneManager.hpp"
#include <wwfcUtil.h>

namespace mkw::System
{

#if RMC

class RKScene
{
public:
    enum class SceneID {
        Menu = 1,
        Race = 2,
        Globe = 4,
    };

private:
    /* 0x000 */ u8 _000[0xC70 - 0x000];
};

static_assert(sizeof(RKScene) == 0xC70);

class RKSystem
{
public:
    EGG::SceneManager* sceneManager() const
    {
        return m_sceneManager;
    }

    static RKSystem& Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ u8 _00[0x54 - 0x00];
    /* 0x54 */ EGG::SceneManager* m_sceneManager;
    /* 0x58 */ u8 _74[0x74 - 0x58];

    static RKSystem& s_instance
        AT(RMCXD_PORT(0x80385FC8, 0x80381C48, 0x80385948, 0x80373FE8));
};

static_assert(sizeof(RKSystem) == 0x74);

#endif

} // namespace mkw::System
