#pragma once

#include "eggSceneManager.hpp"
#include "eggVector.hpp"
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

// https://github.com/riidefi/mkw/blob/master/source/game/system/CourseMap.hpp#L359-L373
class MapdataCannonPoint
{
public:
    enum class CannonType : s16 {
        Direct,
        Curved,
        CurvedSlow,

        Default = -1,
    };

    struct Data {
        EGG::Vector3f position;
        EGG::Vector3f rotation;
        u16 id;
        CannonType cannonType;
    };

    MapdataCannonPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    const Data* m_data;
};

static_assert(sizeof(MapdataCannonPoint) == 0x4);

// https://github.com/riidefi/mkw/blob/master/source/game/system/CourseMap.hpp#L543-L557
class MapdataItemPoint
{
public:
    struct Data {
        EGG::Vector3f position;
        f32 deviation;
        u16 parameters[2];
    };

    MapdataItemPoint(const Data* data)
      : m_data(data)
    {
    }

private:
    const Data* m_data;
    u8 _04[0x14 - 0x04];
};

static_assert(sizeof(MapdataItemPoint) == 0x14);

class RaceConfig
{
public:
    enum GameMode {
        PrivateVersusRace = 7,
        PublicVersusRace = 8,
        PublicBattle = 9,
        PrivateBattle = 10,
    };

    class Scenario
    {
    public:
        bool isOnlineVersusRace() const
        {
            return m_gameMode == GameMode::PrivateVersusRace ||
                   m_gameMode == GameMode::PublicVersusRace;
        }

        bool isOnlineBattle() const
        {
            return m_gameMode == GameMode::PublicBattle ||
                   m_gameMode == GameMode::PrivateBattle;
        }

    private:
        /* 0x000 */ u8 _000[0xB50 - 0x000];
        /* 0xB50 */ GameMode m_gameMode;
        /* 0xB54 */ u8 _B54[0xBF0 - 0xB54];
    };

    static_assert(sizeof(Scenario) == 0xBF0);

    Scenario& raceScenario()
    {
        return m_raceScenario;
    }

    Scenario& menuScenario()
    {
        return m_menuScenario;
    }

    static RaceConfig* Instance()
    {
        return s_instance;
    }

private:
    /* 0x0000 */ u8 _0000[0x0020 - 0x0000];
    /* 0x0020 */ Scenario m_raceScenario;
    /* 0x0C10 */ Scenario m_menuScenario;
    /* 0x1800 */ u8 _1800[0x73F0 - 0x1800];

    static RaceConfig* s_instance
        AT(RMCXD_PORT(0x809BD728, 0x809B8F68, 0x809BC788, 0x809ABD68));
};

static_assert(sizeof(RaceConfig) == 0x73F0);

#endif

} // namespace mkw::System
