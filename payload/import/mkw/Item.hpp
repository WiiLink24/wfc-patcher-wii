#pragma once

#if RMC

#  include <wwfcTypes.h>

namespace wwfc::mkw
{

enum class EItemType {
    KOURA_GREEN = 0x00,
    KOURA_RED = 0x01,
    BANANA = 0x02,
    DUMMY_BOX = 0x03,
    KINOKO = 0x04,
    KINOKO_3 = 0x05,
    BOMHEI = 0x06,
    KOURA_BLUE = 0x07,
    THUNDER = 0x08,
    STAR = 0x09,
    KINOKO_GOLDEN = 0x0A,
    KINOKO_BIG = 0x0B,
    GESSO = 0x0C,
    POW_BLOCK = 0x0D,
    KUMO = 0x0E,
    KILLER = 0x0F,
    KOURA_GREEN_3 = 0x10,
    KOURA_RED_3 = 0x11,
    BANANA_3 = 0x12,

    BASE_COUNT = 0x13,
    EMPTY = 0x14,
};

enum class EItemGeoObjType {
    KOURA_GREEN = 0x00,
    KOURA_RED = 0x01,
    BANANA = 0x02,
    KINOKO = 0x03,
    STAR = 0x04,
    KOURA_BLUE = 0x05,
    THUNDER = 0x06,
    DUMMY_BOX = 0x07,
    KINOKO_BIG = 0x08,
    BOMHEI = 0x09,
    GESSO = 0x0A,
    POW_BLOCK = 0x0B,
    KINOKO_GOLDEN = 0x0C,
    KILLER = 0x0D,
    KUMO = 0x0E,

    BASE_COUNT = 0x0F,
    EMPTY = 0x10,
};

struct ItemBehaviour {
    enum class UseType {
        Use = 0,
        Throw = 1,
        Trail = 2,
        Circle = 3,
    };

    static constexpr UseType s_useType[0x13] = {
        UseType::Throw,  UseType::Throw,  UseType::Throw, UseType::Throw,
        UseType::Use,    UseType::Use,    UseType::Throw, UseType::Throw,
        UseType::Use,    UseType::Use,    UseType::Use,   UseType::Use,
        UseType::Use,    UseType::Use,    UseType::Use,   UseType::Use,
        UseType::Circle, UseType::Circle, UseType::Trail,
    };

    static constexpr bool s_hasUseFunction[0x13] = {
        false, false, false, false, //
        true,  true,  false, true, //
        true,  true,  true,  true, //
        true,  true,  true,  true, //
        false, false, false,
    };
};

static bool IsItemValid(EItemType item)
{
    switch (item) {
    case EItemType::KOURA_GREEN... EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanHoldItem(EItemType item)
{
    switch (item) {
    case EItemType::KOURA_GREEN... EItemType::POW_BLOCK:
    case EItemType::KILLER... EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool DoesItemHaveQuantity(EItemType item)
{
    switch (item) {
    case EItemType::KINOKO_3:
    case EItemType::KOURA_GREEN_3:
    case EItemType::KOURA_RED_3:
    case EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanUseItem(EItemType item)
{
    if (item == EItemType::EMPTY) {
        return false;
    }

    u8 itemToUse = static_cast<u8>(item);

    return ItemBehaviour::s_useType[itemToUse] == ItemBehaviour::UseType::Use;
}

static bool CanThrowItem(EItemType item)
{
    if (item == EItemType::EMPTY) {
        return false;
    }

    u8 itemToThrow = static_cast<u8>(item);

    return ItemBehaviour::s_useType[itemToThrow] ==
           ItemBehaviour::UseType::Throw;
}

static bool CanTrailItem(EItemType item)
{
    if (item == EItemType::EMPTY) {
        return false;
    }

    u8 itemToTrail = static_cast<u8>(item);

    return !ItemBehaviour::s_hasUseFunction[itemToTrail];
}

static bool CanHitItemObject(EItemGeoObjType itemObject)
{
    switch (itemObject) {
    case EItemGeoObjType::KOURA_GREEN... EItemGeoObjType::KILLER: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanItemObjectLockOn(EItemGeoObjType itemObject)
{
    switch (itemObject) {
    case EItemGeoObjType::KOURA_RED:
    case EItemGeoObjType::KOURA_BLUE:
    case EItemGeoObjType::KUMO: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanDropItemObject(EItemGeoObjType itemObject)
{
    switch (itemObject) {
    case EItemGeoObjType::KOURA_GREEN... EItemGeoObjType::KILLER: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsHeldItemValidVS(EItemType item)
{
    switch (item) {
    case EItemType::KOURA_GREEN... EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidVS(EItemType item)
{
    if (!IsHeldItemValidVS(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsHeldItemValidBB(EItemType item)
{
    switch (item) {
    case EItemType::KOURA_GREEN... EItemType::STAR:
    case EItemType::KINOKO_BIG... EItemType::GESSO:
    case EItemType::KOURA_GREEN_3... EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidBB(EItemType item)
{
    if (!IsHeldItemValidBB(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsHeldItemValidCR(EItemType item)
{
    switch (item) {
    case EItemType::KOURA_GREEN... EItemType::KOURA_BLUE:
    case EItemType::STAR... EItemType::POW_BLOCK:
    case EItemType::KOURA_GREEN_3... EItemType::BANANA_3: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidCR(EItemType item)
{
    if (!IsHeldItemValidCR(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsItemObjectValid(EItemGeoObjType itemObject)
{
    switch (itemObject) {
    case EItemGeoObjType::KOURA_GREEN... EItemGeoObjType::KUMO: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static EItemType ItemObjectToItemBox(EItemGeoObjType itemObject)
{
    switch (itemObject) {
    case EItemGeoObjType::KOURA_GREEN: {
        return EItemType::KOURA_GREEN;
    }
    case EItemGeoObjType::KOURA_RED: {
        return EItemType::KOURA_RED;
    }
    case EItemGeoObjType::BANANA: {
        return EItemType::BANANA;
    }
    case EItemGeoObjType::KINOKO: {
        return EItemType::KINOKO;
    }
    case EItemGeoObjType::STAR: {
        return EItemType::STAR;
    }
    case EItemGeoObjType::KOURA_BLUE: {
        return EItemType::KOURA_BLUE;
    }
    case EItemGeoObjType::THUNDER: {
        return EItemType::THUNDER;
    }
    case EItemGeoObjType::DUMMY_BOX: {
        return EItemType::DUMMY_BOX;
    }
    case EItemGeoObjType::KINOKO_BIG: {
        return EItemType::KINOKO_BIG;
    }
    case EItemGeoObjType::BOMHEI: {
        return EItemType::BOMHEI;
    }
    case EItemGeoObjType::GESSO: {
        return EItemType::GESSO;
    }
    case EItemGeoObjType::POW_BLOCK: {
        return EItemType::POW_BLOCK;
    }
    case EItemGeoObjType::KINOKO_GOLDEN: {
        return EItemType::KINOKO_GOLDEN;
    }
    case EItemGeoObjType::KILLER: {
        return EItemType::KILLER;
    }
    case EItemGeoObjType::KUMO: {
        return EItemType::KUMO;
    }
    default: {
        return EItemType::EMPTY;
    }
    }
}

} // namespace wwfc::mkw

#endif // RMC