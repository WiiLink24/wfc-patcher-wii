#pragma once

namespace mkw::Registry
{

enum class Course {
    MarioCircuit = 0x00,
    MooMooMeadows = 0x01,
    MushroomGorge = 0x02,
    GrumbleVolcano = 0x03,
    ToadsFactory = 0x04,
    CoconutMall = 0x05,
    DKSummit = 0x06,
    WarioGoldMine = 0x07,
    LuigiCircuit = 0x08,
    DaisyCircuit = 0x09,
    MoonviewHighway = 0x0A,
    MapleTreeway = 0x0B,
    BowsersCastle = 0x0C,
    RainbowRoad = 0x0D,
    DryDryRuins = 0x0E,
    KoopaCape = 0x0F,
    GCNPeachBeach = 0x10,
    GCNMarioCircuit = 0x11,
    GCNWaluigiStadium = 0x12,
    GCNDKMountain = 0x13,
    DSYoshiFalls = 0x14,
    DSDesertHills = 0x15,
    DSPeachGardens = 0x16,
    DSDelfinoSquare = 0x17,
    SNESMarioCircuit3 = 0x18,
    SNESGhostValley2 = 0x19,
    N64MarioRaceway = 0x1A,
    N64SherbetLand = 0x1B,
    N64BowsersCastle = 0x1C,
    N64DKsJungleParkway = 0x1D,
    GBABowserCastle3 = 0x1E,
    GBAShyGuyBeach = 0x1F,
    DelfinoPier = 0x20,
    BlockPlaza = 0x21,
    ChainChompRoulette = 0x22,
    FunkyStadium = 0x23,
    ThwompDesert = 0x24,
    GCNCookieLand = 0x25,
    DSTwilightHouse = 0x26,
    SNESBattleCourse4 = 0x27,
    GBABattleCourse3 = 0x28,
    N64Skyscraper = 0x29,
};

static constexpr bool IsRaceCourse(Course course)
{
    return course >= Course::MarioCircuit && course <= Course::GBAShyGuyBeach;
}

static constexpr bool IsBattleCourse(Course course)
{
    return course >= Course::DelfinoPier && course <= Course::N64Skyscraper;
}

} // namespace mkw::Registry
