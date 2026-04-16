#pragma once

#if RMC

namespace wwfc::mkw
{

enum class ECharacter : int {
    MARIO         = 0x00,
    BABY_PEACH    = 0x01,
    WALUIGI       = 0x02,
    BOWSER        = 0x03,
    BABY_DAISY    = 0x04,
    DRY_BONES     = 0x05,
    BABY_MARIO    = 0x06,
    LUIGI         = 0x07,
    TOAD          = 0x08,
    DONKEY_KONG   = 0x09,
    YOSHI         = 0x0A,
    WARIO         = 0x0B,
    BABY_LUIGI    = 0x0C,
    TOADETTE      = 0x0D,
    KOOPA_TROOPA  = 0x0E,
    DAISY         = 0x0F,
    PEACH         = 0x10,
    BIRDO         = 0x11,
    DIDDY_KONG    = 0x12,
    KING_BOO      = 0x13,
    BOWSER_JR     = 0x14,
    DRY_BOWSER    = 0x15,
    FUNKY_KONG    = 0x16,
    ROSALINA      = 0x17,
    S_MII_AM      = 0x18,
    S_MII_AF      = 0x19,
    S_MII_BM      = 0x1A,
    S_MII_BF      = 0x1B,
    S_MII_CM      = 0x1C,
    S_MII_CF      = 0x1D,
    M_MII_AM      = 0x1E,
    M_MII_AF      = 0x1F,
    M_MII_BM      = 0x20,
    M_MII_BF      = 0x21,
    M_MII_CM      = 0x22,
    M_MII_CF      = 0x23,
    L_MII_AM      = 0x24,
    L_MII_AF      = 0x25,
    L_MII_BM      = 0x26,
    L_MII_BF      = 0x27,
    L_MII_CM      = 0x28,
    L_MII_CF      = 0x29,
    M_MII         = 0x2A,
    S_MII         = 0x2B,
    L_MII         = 0x2C,
    PEACH_MENU    = 0x2D,
    DAISY_MENU    = 0x2E,
    ROSALINA_MENU = 0x2F,

    // Control IDs
    CONTROL_VOTING_NOT_SELECTED = 0x30,
};

enum class EVehicle : int {
    S_STANDARD_KART    = 0x00,
    M_STANDARD_KART    = 0x01,
    L_STANDARD_KART    = 0x02,
    S_BOOSTER_SEAT     = 0x03,
    M_CLASSIC_DRAGSTER = 0x04,
    L_OFFROADER        = 0x05,
    S_MINI_BEAST       = 0x06,
    M_WILD_WING        = 0x07,
    L_FLAME_FLYER      = 0x08,
    S_CHEEP_CHARGER    = 0x09,
    M_SUPER_BLOOPER    = 0x0A,
    L_PIRANHA_PROWLER  = 0x0B,
    S_TINY_TITAN       = 0x0C,
    M_DAYTRIPPER       = 0x0D,
    L_JETSETTER        = 0x0E,
    S_BLUE_FALCON      = 0x0F,
    M_SPRINTER         = 0x10,
    L_HONEYCOUPE       = 0x11,
    S_STANDARD_BIKE    = 0x12,
    M_STANDARD_BIKE    = 0x13,
    L_STANDARD_BIKE    = 0x14,
    S_BULLET_BIKE      = 0x15,
    M_MACH_BIKE        = 0x16,
    L_FLAME_RUNNER     = 0x17,
    S_BIT_BIKE         = 0x18,
    M_SUGARSCOOT       = 0x19,
    L_WARIO_BIKE       = 0x1A,
    S_QUACKER          = 0x1B,
    M_ZIP_ZIP          = 0x1C,
    L_SHOOTING_STAR    = 0x1D,
    S_MAGIKRUISER      = 0x1E,
    M_SNEAKSTER        = 0x1F,
    L_SPEAR            = 0x20,
    S_JET_BUBBLE       = 0x21,
    M_DOLPHIN_DASHER   = 0x22,
    L_PHANTOM          = 0x23,

    // Control IDs
    CONTROL_VOTING_NOT_SELECTED = 0x24,
};

enum class ECup : int {
    KINOKO    = 0x0,
    FLOWER    = 0x1,
    STAR      = 0x2,
    SPECIAL   = 0x3,
    KOURA     = 0x4,
    BANANA    = 0x5,
    KONOHA    = 0x6,
    LIGHTNING = 0x7,

    BATTLE_WII   = 0x0,
    BATTLE_RETRO = 0x1,
};

enum class ECourse : int {
    // Mushroom Cup
    LUIGI_CIRCUIT   = 0x08,
    MOO_MOO_MEADOWS = 0x01,
    MUSHROOM_GORGE  = 0x02,
    TOADS_FACTORY   = 0x04,

    // Flower Cup
    MARIO_CIRCUIT   = 0x00,
    COCONUT_MALL    = 0x05,
    DK_SUMMIT       = 0x06,
    WARIO_GOLD_MINE = 0x07,

    // Star Cup
    DAISY_CIRCUIT   = 0x09,
    KOOPA_CAPE      = 0x0F,
    MAPLE_TREEWAY   = 0x0B,
    GRUMBLE_VOLCANO = 0x03,

    // Special Cup
    DRY_DRY_RUINS    = 0x0E,
    MOONVIEW_HIGHWAY = 0x0A,
    BOWSER_CASTLE    = 0x0C,
    RAINBOW_ROAD     = 0x0D,

    // Shell Cup
    GCN_PEACH_BEACH     = 0x10,
    DS_YOSHI_FALLS      = 0x14,
    SNES_GHOST_VALLEY_2 = 0x19,
    N64_MARIO_RACEWAY   = 0x1A,

    // Banana Cup
    N64_SHERBET_LAND    = 0x1B,
    GBA_SHY_GUY_BEACH   = 0x1F,
    DS_DELFINO_SQUARE   = 0x17,
    GCN_WALUIGI_STADIUM = 0x12,

    // Leaf Cup
    DS_DESERT_HILLS       = 0x15,
    GBA_BOWSER_CASTLE_3   = 0x1E,
    N64_DK_JUNGLE_PARKWAY = 0x1D,
    GCN_MARIO_CIRCUIT     = 0x11,

    // Lightning Cup
    SNES_MARIO_CIRCUIT_3 = 0x18,
    DS_PEACH_GARDENS     = 0x16,
    GCN_DK_MOUNTAIN      = 0x13,
    N64_BOWSER_CASTLE    = 0x1C,

    // Battle Wii Cup
    BLOCK_PLAZA          = 0x21,
    DELFINO_PIER         = 0x20,
    FUNKY_STADIUM        = 0x23,
    CHAIN_CHOMP_ROULETTE = 0x22,
    THWOMP_DESERT        = 0x24,

    // Battle Retro Cup
    SNES_BATTLE_COURSE_4 = 0x27,
    GBA_BATTLE_COURSE_3  = 0x28,
    N64_SKYSCRAPER       = 0x29,
    GCN_COOKIE_LAND      = 0x25,
    DS_TWILIGHT_HOUSE    = 0x26,

    // Special
    GALAXY_ARENA         = 0x36,
    SUNSET_LUIGI_CIRCUIT = 0x3A,

    // Demo
    DEMO_WIN  = 0x37,
    DEMO_LOSE = 0x38,
    DEMO_DRAW = 0x39,

    // Control IDs
    CONTROL_VOTING_01           = 0x42,
    CONTROL_VOTING_NOT_SELECTED = 0x43,
    CONTROL_VOTING_RANDOM       = 0xFF,
    CONTROL_VOTING_NOT_DECIDED  = 0xFF,
};

[[gnu::longcall]] int GetVehicleWeightClass(EVehicle vehicle) AT(
    RMCXD_PORT(0x8081CB70, 0x80809DC4, 0x8081C1DC, 0x8080AF30, 0x8081D11C)
);
[[gnu::longcall]] int GetCharacterWeightClass(ECharacter character) AT(
    RMCXD_PORT(0x8081CD3C, 0x80809F90, 0x8081C3A8, 0x8080B0FC, 0x8081D2E8)
);

static bool IsCharacterValid(ECharacter character)
{
    switch (character) {
    case ECharacter::MARIO... ECharacter::S_MII_BF:
    case ECharacter::M_MII_AM... ECharacter::M_MII_BF:
    case ECharacter::L_MII_AM... ECharacter::L_MII_BF:
        return true;
    default:
        return false;
    }
}

static bool IsVehicleValidVS(EVehicle vehicle)
{
    return vehicle >= EVehicle::S_STANDARD_KART && vehicle <= EVehicle::L_PHANTOM;
}

static bool IsVehicleValidBT(EVehicle vehicle)
{
    switch (vehicle) {
    case EVehicle::S_STANDARD_KART... EVehicle::L_STANDARD_KART:
    case EVehicle::S_STANDARD_BIKE... EVehicle::L_STANDARD_BIKE:
        return true;
    default:
        return false;
    }
}

static bool IsCombinationValidVS(ECharacter character, EVehicle vehicle)
{
    if (!IsCharacterValid(character)) {
        return false;
    }
    if (!IsVehicleValidVS(vehicle)) {
        return false;
    }

    return GetCharacterWeightClass(character) == GetVehicleWeightClass(vehicle);
}

static bool IsCombinationValidBT(ECharacter character, EVehicle vehicle)
{
    if (!IsCharacterValid(character)) {
        return false;
    }
    if (!IsVehicleValidBT(vehicle)) {
        return false;
    }

    return GetCharacterWeightClass(character) == GetVehicleWeightClass(vehicle);
}

static bool IsRaceCourse(ECourse course)
{
    return course >= ECourse::MARIO_CIRCUIT && course <= ECourse::GBA_SHY_GUY_BEACH;
}

static bool IsBattleCourse(ECourse course)
{
    return course >= ECourse::DELFINO_PIER && course <= ECourse::N64_SKYSCRAPER;
}

} // namespace wwfc::mkw

#endif // RMC