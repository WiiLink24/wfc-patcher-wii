#pragma once

#include <wwfcUtil.h>

namespace mkw::Registry
{

#if RMC

enum class Character {
    Mario = 0x00,
    BabyPeach = 0x01,
    Waluigi = 0x02,
    Bowser = 0x03,
    BabyDaisy = 0x04,
    DryBones = 0x05,
    BabyMario = 0x06,
    Luigi = 0x07,
    Toad = 0x08,
    DonkeyKong = 0x09,
    Yoshi = 0x0A,
    Wario = 0x0B,
    BabyLuigi = 0x0C,
    Toadette = 0x0D,
    KoopaTroopa = 0x0E,
    Daisy = 0x0F,
    Peach = 0x10,
    Birdo = 0x11,
    DiddyKong = 0x12,
    KingBoo = 0x13,
    BowserJr = 0x14,
    DryBowser = 0x15,
    FunkyKong = 0x16,
    Rosalina = 0x17,
    SmallMiiOutfitAMale = 0x18,
    SmallMiiOutfitAFemale = 0x19,
    SmallMiiOutfitBMale = 0x1A,
    SmallMiiOutfitBFemale = 0x1B,
    MediumMiiOutfitAMale = 0x1E,
    MediumMiiOutfitAFemale = 0x1F,
    MediumMiiOutfitBMale = 0x20,
    MediumMiiOutfitBFemale = 0x21,
    LargeMiiOutfitAMale = 0x24,
    LargeMiiOutfitAFemale = 0x25,
    LargeMiiOutfitBMale = 0x26,
    LargeMiiOutfitBFemale = 0x27,
};

enum class Vehicle {
    StandardKartSmall = 0x00,
    StandardKartMedium = 0x01,
    StandardKartLarge = 0x02,
    BoosterSeat = 0x03,
    ClassicDragster = 0x04,
    Offroader = 0x05,
    MiniBeast = 0x06,
    WildWing = 0x07,
    FlameFlyer = 0x08,
    CheepCharger = 0x09,
    SuperBlooper = 0x0A,
    PiranhaProwler = 0x0B,
    TinyTitan = 0x0C,
    Daytripper = 0x0D,
    Jetsetter = 0x0E,
    BlueFalcon = 0x0F,
    Sprinter = 0x10,
    Honeycoupe = 0x11,
    StandardBikeSmall = 0x12,
    StandardBikeMedium = 0x13,
    StandardBikeLarge = 0x14,
    BulletBike = 0x15,
    MachBike = 0x16,
    FlameRunner = 0x17,
    BitBike = 0x18,
    Sugarscoot = 0x19,
    WarioBike = 0x1A,
    Quacker = 0x1B,
    ZipZip = 0x1C,
    ShootingStar = 0x1D,
    Magikruiser = 0x1E,
    Sneakster = 0x1F,
    Spear = 0x20,
    JetBubble = 0x21,
    DolphinDasher = 0x22,
    Phantom = 0x23,
};

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

LONGCALL int GetVehicleWeightClass(Vehicle vehicle)
    AT(RMCXD_PORT(0x8081CB70, 0x80809DC4, 0x8081C1DC, 0x8080AF30));
LONGCALL int GetCharacterWeightClass(Character character)
    AT(RMCXD_PORT(0x8081CD3C, 0x80809F90, 0x8081C3A8, 0x8080B0FC));

static bool IsCharacterValid(Character character)
{
    switch (character) {
    case Character::Mario... Character::SmallMiiOutfitBFemale:
    case Character::MediumMiiOutfitAMale... Character::MediumMiiOutfitBFemale:
    case Character::LargeMiiOutfitAMale... Character::LargeMiiOutfitBFemale:
        return true;
    default:
        return false;
    }
}

static bool IsVehicleValidVS(Vehicle vehicle)
{
    return vehicle >= Vehicle::StandardKartSmall && vehicle <= Vehicle::Phantom;
}

static bool IsVehicleValidBT(Vehicle vehicle)
{
    switch (vehicle) {
    case Vehicle::StandardKartSmall... Vehicle::StandardKartLarge:
    case Vehicle::StandardBikeSmall... Vehicle::StandardBikeLarge:
        return true;
    default:
        return false;
    }
}

static bool IsCombinationValidVS(Character character, Vehicle vehicle)
{
    if (!IsCharacterValid(character)) {
        return false;
    }
    if (!IsVehicleValidVS(vehicle)) {
        return false;
    }

    return GetCharacterWeightClass(character) == GetVehicleWeightClass(vehicle);
}

static bool IsCombinationValidBT(Character character, Vehicle vehicle)
{
    if (!IsCharacterValid(character)) {
        return false;
    }
    if (!IsVehicleValidBT(vehicle)) {
        return false;
    }

    return GetCharacterWeightClass(character) == GetVehicleWeightClass(vehicle);
}

static bool IsRaceCourse(Course course)
{
    return course >= Course::MarioCircuit && course <= Course::GBAShyGuyBeach;
}

static bool IsBattleCourse(Course course)
{
    return course >= Course::DelfinoPier && course <= Course::N64Skyscraper;
}

#endif

} // namespace mkw::Registry
