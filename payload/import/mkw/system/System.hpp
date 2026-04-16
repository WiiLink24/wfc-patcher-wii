#pragma once

#if RMC

#  include "import/egg/heap.hpp"
#  include "import/egg/sceneManager.hpp"

namespace wwfc::mkw
{

class Scene
{
public:
    enum class ESceneID {
        MENU  = 1,
        RACE  = 2,
        GLOBE = 4,
    };

private:
    /* 0x000 */ u8 _000[0xC70 - 0x000];
};

static_assert(sizeof(Scene) == 0xC70);

class System
{
public:
    static System& Instance()
    {
        return s_instance;
    }

    EGG::Heap* getSystemHeap() const
    {
        return m_systemHeap;
    }

    EGG::SceneManager* getSceneManager() const
    {
        return m_sceneManager;
    }

    Scene::ESceneID getCurrentSceneID()
    {
        return static_cast<Scene::ESceneID>(getSceneManager()->getCurrentSceneID());
    }

    bool isCurrentSceneID(Scene::ESceneID sceneId)
    {
        return getCurrentSceneID() == sceneId;
    }

private:
    /* 0x00 */ u8                 _00[0x24 - 0x00];
    /* 0x24 */ EGG::Heap*         m_systemHeap;
    /* 0x28 */ u8                 _28[0x54 - 0x28];
    /* 0x54 */ EGG::SceneManager* m_sceneManager;
    /* 0x58 */ u8                 _74[0x74 - 0x58];

    static System&
        s_instance AT(RMCXD_PORT(0x80385FC8, 0x80381C48, 0x80385948, 0x80373FE8, 0x80385648));
};

static_assert(sizeof(System) == 0x74);

} // namespace wwfc::mkw

#endif // RMC