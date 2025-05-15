#pragma once

#if RMC

#  include "import/egg/heap.hpp"
#  include "import/egg/sceneManager.hpp"

namespace wwfc::mkw::System
{

class Scene
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

static_assert(sizeof(Scene) == 0xC70);

class System
{
public:
    EGG::Heap* systemHeap() const
    {
        return m_systemHeap;
    }

    EGG::SceneManager* sceneManager() const
    {
        return m_sceneManager;
    }

    static System& Instance()
    {
        return s_instance;
    }

private:
    /* 0x00 */ u8 _00[0x24 - 0x00];
    /* 0x24 */ EGG::Heap* m_systemHeap;
    /* 0x28 */ u8 _28[0x54 - 0x28];
    /* 0x54 */ EGG::SceneManager* m_sceneManager;
    /* 0x58 */ u8 _74[0x74 - 0x58];

    static System& s_instance
        AT(RMCXD_PORT(0x80385FC8, 0x80381C48, 0x80385948, 0x80373FE8));
};

static_assert(sizeof(System) == 0x74);

} // namespace wwfc::mkw::System

#endif // RMC