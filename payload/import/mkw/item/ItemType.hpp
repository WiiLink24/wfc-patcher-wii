#pragma once

#if RMC

#  include <wwfcTypes.h>

namespace wwfc::mkw
{

enum class EItemType : unsigned int {
    KOURA_GREEN   = 0x00, // Green Shell
    KOURA_RED     = 0x01, // Red Shell
    BANANA        = 0x02, // Banana
    DUMMY_BOX     = 0x03, // Fake Item Box
    KINOKO        = 0x04, // Mushroom
    KINOKO_3      = 0x05, // Triple Mushroom
    BOMHEI        = 0x06, // Bob-omb
    KOURA_BLUE    = 0x07, // Spiny Shell
    THUNDER       = 0x08, // Lightning
    STAR          = 0x09, // Star
    KINOKO_GOLDEN = 0x0A, // Golden Mushroom
    KINOKO_BIG    = 0x0B, // Mega Mushroom
    GESSO         = 0x0C, // Blooper
    POW_BLOCK     = 0x0D, // POW Block
    KUMO          = 0x0E, // Lightning Cloud
    KILLER        = 0x0F, // Bullet Bill
    KOURA_GREEN_3 = 0x10, // Triple Green Shell
    KOURA_RED_3   = 0x11, // Triple Red Shell
    BANANA_3      = 0x12, // Triple Banana
    COUNT         = 0x13, // Not used, invalid
    EMPTY         = 0x14, // No Item
};

enum class EItemGeoObjType : unsigned int {
    KOURA_GREEN   = 0x00, // Green Shell
    KOURA_RED     = 0x01, // Red Shell
    BANANA        = 0x02, // Banana
    KINOKO        = 0x03, // Mushroom
    STAR          = 0x04, // Star
    KOURA_BLUE    = 0x05, // Spiny Shell
    THUNDER       = 0x06, // Lightning
    DUMMY_BOX     = 0x07, // Fake Item Box
    KINOKO_BIG    = 0x08, // Mega Mushroom
    BOMHEI        = 0x09, // Bob-omb
    GESSO         = 0x0A, // Blooper
    POW_BLOCK     = 0x0B, // POW Block
    KINOKO_GOLDEN = 0x0C, // Golden Mushroom
    KILLER        = 0x0D, // Bullet Bill
    KUMO          = 0x0E, // Lightning Cloud
    COUNT         = 0x0F, // Not used, invalid
    EMPTY         = 0x10, // No Item
};

enum class EItemTailType {
    NONE   = 0, // e.g. Bullet Bill
    HOLD   = 1, // e.g. Banana
    TAIL   = 2, // e.g. Triple Banana
    CIRCLE = 3, // e.g. Triple Shell
};

struct ItemDefaults {
    EItemTailType m_tailType : 2;
    bool          m_inVS : 1;
    bool          m_inBTBalloon : 1;
    bool          m_inBTCoin : 1;
    bool          m_hasUseMethod : 1;
    bool          m_hasQuantity : 1;

    static ItemDefaults s_table[];

    static bool isValid(EItemType item);
    static bool canHold(EItemType item);
    static bool hasQuantity(EItemType item);
    static bool canUse(EItemType item);
    static bool canThrow(EItemType item);
    static bool canTrail(EItemType item);
    static bool isValidVS(EItemType item);
    static bool isTrailValidVS(EItemType item);
    static bool isValidBTBalloon(EItemType item);
    static bool isTrailValidBTBalloon(EItemType item);
    static bool isValidBTCoin(EItemType item);
    static bool isTrailValidBTCoin(EItemType item);
    static bool isObjectValid(EItemGeoObjType itemObject);

    static bool canHitObject(EItemGeoObjType itemObject);
    static bool canObjectLockOn(EItemGeoObjType itemObject);
    static bool canDropObject(EItemGeoObjType itemObject);

    static EItemType ItemTypeFromGeoObjType(EItemGeoObjType itemObject);
};

inline ItemDefaults ItemDefaults::s_table[u8(EItemType::COUNT)] = {
    [u8(EItemType::KOURA_GREEN)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KOURA_RED)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::BANANA)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::DUMMY_BOX)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KINOKO)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KINOKO_3)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = true,
    },
    [u8(EItemType::BOMHEI)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KOURA_BLUE)] = {
        .m_tailType     = EItemTailType::HOLD,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::THUNDER)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = false,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::STAR)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KINOKO_GOLDEN)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = false,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KINOKO_BIG)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::GESSO)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::POW_BLOCK)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = false,
        .m_inBTCoin     = true,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KUMO)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = false,
        .m_inBTCoin     = false,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KILLER)] = {
        .m_tailType     = EItemTailType::NONE,
        .m_inVS         = true,
        .m_inBTBalloon  = false,
        .m_inBTCoin     = false,
        .m_hasUseMethod = true,
        .m_hasQuantity  = false,
    },
    [u8(EItemType::KOURA_GREEN_3)] = {
        .m_tailType     = EItemTailType::CIRCLE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = true,
    },
    [u8(EItemType::KOURA_RED_3)] = {
        .m_tailType     = EItemTailType::CIRCLE,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = true,
    },
    [u8(EItemType::BANANA_3)] = {
        .m_tailType     = EItemTailType::TAIL,
        .m_inVS         = true,
        .m_inBTBalloon  = true,
        .m_inBTCoin     = true,
        .m_hasUseMethod = false,
        .m_hasQuantity  = true,
    },
};

inline bool ItemDefaults::isValid(EItemType item)
{
    return item >= EItemType::KOURA_GREEN && item < EItemType::COUNT;
}

inline bool ItemDefaults::canHold(EItemType item)
{
    return isValid(item) && item != EItemType::KUMO;
}

inline bool ItemDefaults::hasQuantity(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_hasQuantity;
}

inline bool ItemDefaults::canUse(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_tailType == EItemTailType::NONE;
}

inline bool ItemDefaults::canThrow(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_tailType == EItemTailType::HOLD;
}

inline bool ItemDefaults::canTrail(EItemType item)
{
    return isValid(item) && !s_table[int(item)].m_hasUseMethod;
}

inline bool ItemDefaults::isValidVS(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_inVS;
}

inline bool ItemDefaults::isTrailValidVS(EItemType item)
{
    return isValidVS(item) && canTrail(item);
}

inline bool ItemDefaults::isValidBTBalloon(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_inBTBalloon;
}

inline bool ItemDefaults::isTrailValidBTBalloon(EItemType item)
{
    return isValidBTBalloon(item) && canTrail(item);
}

inline bool ItemDefaults::isValidBTCoin(EItemType item)
{
    return isValid(item) && s_table[int(item)].m_inBTCoin;
}

inline bool ItemDefaults::isTrailValidBTCoin(EItemType item)
{
    return isValidBTCoin(item) && canTrail(item);
}

inline bool ItemDefaults::isObjectValid(EItemGeoObjType itemObject)
{
    return itemObject >= EItemGeoObjType::KOURA_GREEN && itemObject < EItemGeoObjType::COUNT;
}

inline bool ItemDefaults::canHitObject(EItemGeoObjType itemObject)
{
    return isObjectValid(itemObject) && itemObject != EItemGeoObjType::KUMO;
}

inline bool ItemDefaults::canObjectLockOn(EItemGeoObjType itemObject)
{
    return isObjectValid(itemObject) &&
           (itemObject == EItemGeoObjType::KOURA_RED || itemObject == EItemGeoObjType::KOURA_BLUE ||
            itemObject == EItemGeoObjType::KUMO);
}

inline bool ItemDefaults::canDropObject(EItemGeoObjType itemObject)
{
    return isObjectValid(itemObject) && itemObject != EItemGeoObjType::KUMO;
}

inline EItemType ItemDefaults::ItemTypeFromGeoObjType(EItemGeoObjType itemObject)
{
    static constexpr EItemType s_conversion[] = {
        [u8(EItemGeoObjType::KOURA_GREEN)]   = EItemType::KOURA_GREEN,
        [u8(EItemGeoObjType::KOURA_RED)]     = EItemType::KOURA_RED,
        [u8(EItemGeoObjType::BANANA)]        = EItemType::BANANA,
        [u8(EItemGeoObjType::KINOKO)]        = EItemType::KINOKO,
        [u8(EItemGeoObjType::STAR)]          = EItemType::STAR,
        [u8(EItemGeoObjType::KOURA_BLUE)]    = EItemType::KOURA_BLUE,
        [u8(EItemGeoObjType::THUNDER)]       = EItemType::THUNDER,
        [u8(EItemGeoObjType::DUMMY_BOX)]     = EItemType::DUMMY_BOX,
        [u8(EItemGeoObjType::KINOKO_BIG)]    = EItemType::KINOKO_BIG,
        [u8(EItemGeoObjType::BOMHEI)]        = EItemType::BOMHEI,
        [u8(EItemGeoObjType::GESSO)]         = EItemType::GESSO,
        [u8(EItemGeoObjType::POW_BLOCK)]     = EItemType::POW_BLOCK,
        [u8(EItemGeoObjType::KINOKO_GOLDEN)] = EItemType::KINOKO_GOLDEN,
        [u8(EItemGeoObjType::KILLER)]        = EItemType::KILLER,
        [u8(EItemGeoObjType::KUMO)]          = EItemType::KUMO,
    };

    if (!isObjectValid(itemObject)) {
        return EItemType::EMPTY;
    }
    return s_conversion[static_cast<u8>(itemObject)];
}

} // namespace wwfc::mkw

#endif // RMC
