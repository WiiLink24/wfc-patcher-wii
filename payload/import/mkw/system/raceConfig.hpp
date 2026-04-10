#pragma once

#if RMC

namespace wwfc::mkw::System
{

class RaceConfig
{
public:
    enum GameMode {
        PrivateVersusRace = 7,
        PublicVersusRace = 8,
        PublicBattle = 9,
        PrivateBattle = 10,
    };

    enum BattleType {
        BalloonBattle = 0,
        CoinRunners = 1,
    };

    class Player
    {
    public:
        enum class Type {
            Master = 0, // a.k.a RealLocal
            CPU = 1,
            Unknown = 2,
            Ghost = 3,
            Remote = 4,
            None = 5,
        };

        FILL(0x00, 0x05);
        /* 0x05 */ u8 m_localPlayerId;
        FILL(0x06, 0x10);
        /* 0x10 */ Type m_type;
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

        bool isBalloonBattle() const
        {
            return m_battleType == BattleType::BalloonBattle;
        }

        bool isCoinRunners() const
        {
            return m_battleType == BattleType::CoinRunners;
        }

        Player* getPlayer(int index)
        {
            [[gnu::longcall]] Player* getPlayer(Scenario * scenario, int index)
                AT(RMCXD_PORT(
                    0x8052E434, 0x805298EC, 0x8052DDB4, 0x8051C48C, 0x8052DC50
                ));
            return getPlayer(this, index);
        }

    private:
        /* 0x000 */ u8 _000[0xB50 - 0x000];
        /* 0xB50 */ GameMode m_gameMode;
        /* 0xB54 */ u8 _B54[0xB58 - 0xB54];
        /* 0xB58 */ BattleType m_battleType;
        /* 0xB5C */ u8 _B5C[0xBF0 - 0xB5C];
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

    static RaceConfig* s_instance AT(
        RMCXD_PORT(0x809BD728, 0x809B8F68, 0x809BC788, 0x809ABD68, 0x809BDFA8) //Please Check The Demo Port
    );
};

static_assert(sizeof(RaceConfig) == 0x73F0);

} // namespace wwfc::mkw::System

#endif