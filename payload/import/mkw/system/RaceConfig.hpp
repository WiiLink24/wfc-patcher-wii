#pragma once

#if RMC

#  include "../Registry.hpp"
#  include "PlayerRating.hpp"

namespace wwfc::mkw
{

class RaceConfigPlayer
{
public:
    enum class ETeam : int {
        TEAM_RED  = 0x0,
        TEAM_BLUE = 0x1,
        TEAM_NONE = 0x2,
    };

    enum class EPlayerType : int {
        MASTER  = 0, // a.k.a RealLocal
        CPU     = 1,
        UNKNOWN = 2,
        GHOST   = 3,
        REMOTE  = 4,
        NONE    = 5,
    };

    FILL(0x00, 0x04);
    /* 0x04 */ u8           m_0x04;
    /* 0x05 */ u8           m_screenId;
    /* 0x06 */ u8           m_inputIndex;
    /* 0x08 */ EVehicle     m_vehicle;
    /* 0x0C */ ECharacter   m_character;
    /* 0x10 */ EPlayerType  m_type;
    /* 0x14 */ u8           m_mii[0xB8];
    /* 0xCC */ ETeam        m_team;
    /* 0xD0 */ s32          m_controllerId;
    /* 0xD4 */ u32          m_0xD4;
    /* 0xD8 */ u16          m_prevScore;
    /* 0xDA */ u16          m_score;
    /* 0xDC */ u16          m_0xDC;
    /* 0xDE */ u16          m_gpRankScore;
    /* 0xE0 */ u8           m_gpRank;
    /* 0xE1 */ u8           m_startPos;
    /* 0xE4 */ PlayerRating m_rating;
    /* 0xEC */ u8           m_0xEC;
};

class RaceConfig
{
public:
    using Player = RaceConfigPlayer;

    enum class EEngineClass : int {
        CC_50  = 0,
        CC_100 = 1,
        CC_150 = 2,
        // Note: Battle mode actually sets it to 50cc (which is ignored by
        // code), but setting it to this in other modes results in Battle CC
        CC_BATTLE = 3,
    };

    enum class EGameMode : int {
        GRAND_PRIX         = 0,
        VS_RACE            = 1,
        TIME_ATTACK        = 2,
        BATTLE             = 3,
        MISSION_RUN        = 4,
        GHOST_RACE         = 5,
        WIFI_FRIEND_VS     = 7,
        WIFI_PUBLIC_VS     = 8,
        WIFI_PUBLIC_BATTLE = 9,
        WIFI_FRIEND_BATTLE = 10,
        AWARD              = 11,
        CREDITS            = 12,
    };

    enum class ECameraMode : int {
        GAMEPLAY_NO_INTRO = 0,
        REPLAY            = 1,
        TITLE_ONE_PLAYER  = 2,
        TITLE_TWO_PLAYER  = 3,
        TITLE_FOUR_PLAYER = 4,
        GAMEPLAY_INTRO    = 5,
        LIVE_VIEW         = 6,
        GRAND_PRIX_WIN    = 7,
        SOLO_VS_WIN       = 8,
        TEAM_VS_WIN       = 9,
        BATTLE_WIN        = 10,
        UNK_11            = 11,
        LOSS              = 12,
    };

    enum class EBattleMode : int {
        BALLOON = 0,
        COIN    = 1,
    };

    enum class ECPUMode : int {
        EASY   = 0,
        NORMAL = 1,
        HARD   = 2,
        NONE   = 3,
    };

    enum class EItemMode : int {
        BALANCED  = 0x0,
        FRANTIC   = 0x1,
        STRATEGIC = 0x2,
        NONE      = 0x3,
    };

    enum ModeFlags : int {
        MODE_FLAGS_MIRROR      = 1 << 0,
        MODE_FLAGS_TEAMS       = 1 << 1,
        MODE_FLAGS_COMPETITION = 1 << 2,
    };

    virtual ~RaceConfig() = default;

    bool isWifiVSRace() const
    {
        return m_gameMode == EGameMode::WIFI_FRIEND_VS || m_gameMode == EGameMode::WIFI_PUBLIC_VS;
    }

    bool isWifiBattle() const
    {
        return m_gameMode == EGameMode::WIFI_PUBLIC_BATTLE ||
               m_gameMode == EGameMode::WIFI_FRIEND_BATTLE;
    }

    bool isBattleBalloon() const
    {
        return m_battleMode == EBattleMode::BALLOON;
    }

    bool isBattleCoin() const
    {
        return m_battleMode == EBattleMode::COIN;
    }

    RaceConfigPlayer* getPlayer(int index)
    {
        [[gnu::longcall]] RaceConfigPlayer* getPlayer(RaceConfig * scenario, int index)
            AT(RMCXD_PORT(0x8052E434, 0x805298EC, 0x8052DDB4, 0x8051C48C, 0x8052DC50));
        return getPlayer(this, index);
    }

private:
    /* 0x004 */ u8               m_playerCount;
    /* 0x005 */ u8               m_screenCount;
    /* 0x006 */ u8               m_localPlayerCount;
    /* 0x007 */ u8               m_screenCountRace;
    /* 0x008 */ RaceConfigPlayer m_players[12];
    /* 0xB48 */ ECourse          m_course;
    /* 0xB4C */ EEngineClass     m_engineClass;
    /* 0xB50 */ EGameMode        m_gameMode;
    /* 0xB54 */ ECameraMode      m_cameraMode;
    /* 0xB58 */ EBattleMode      m_battleMode;
    /* 0xB5C */ ECPUMode         m_cpuMode;
    /* 0xB60 */ EItemMode        m_itemMode;
    /* 0xB64 */ u8               m_screenPlayerIds[4];
    /* 0xB68 */ ECup             m_cup;
    /* 0xB6C */ u8               m_raceNum;
    /* 0xB6D */ u8               m_lapCount;
    /* 0xB70 */ u32              m_modeFlags;
    // Must be fixed for a specific race to replay properly
    /* 0xB74 */ u32 m_seedFixed;
    // Can change between race and replay
    /* 0xB78 */ u32 m_seedRandom;
    // Mission info, ghost pointer
    FILL(0xB7C, 0xBF0);
};

static_assert(sizeof(RaceConfig) == 0xBF0);

class RaceConfigManager
{
public:
    constexpr RaceConfig& getConfig()
    {
        return m_conf;
    }

    constexpr RaceConfig& getConfigNext()
    {
        return m_confNext;
    }

    constexpr RaceConfig& getConfigAward()
    {
        return m_confAward;
    }

    static RaceConfigManager* Instance()
    {
        return s_instance;
    }

    void loadNextCourse()
    {
        [[gnu::longcall]] void loadNextCourse( //
            RaceConfigManager* self
        ) AT(RMCXD_PORT(0x80531F80, 0x8052D438, 0x80531900, 0x8051FFD8, 0x8053179C));
        return loadNextCourse(this);
    }

private:
    // ParameterFile, dummy vtable
    FILL(0x00, 0x20);

    /* 0x0020 */ RaceConfig m_conf;
    /* 0x0C10 */ RaceConfig m_confNext;
    /* 0x1800 */ RaceConfig m_confAward;

    // Raw ghost data
    FILL(0x23F0, 0x73F0);

    static RaceConfigManager*
        s_instance AT(RMCXD_PORT(0x809BD728, 0x809B8F68, 0x809BC788, 0x809ABD68, 0x809BDFA8));
};

static_assert(sizeof(RaceConfigManager) == 0x73F0);

} // namespace wwfc::mkw

#endif
