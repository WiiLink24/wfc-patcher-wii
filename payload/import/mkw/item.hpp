#pragma once

#if RMC

#  include <wwfcCommon.h>

namespace wwfc::mkw::Item
{

enum class ItemBox {
    GreenShell = 0x00,
    RedShell = 0x01,
    Banana = 0x02,
    FakeItemBox = 0x03,
    Mushroom = 0x04,
    TripleMushrooms = 0x05,
    Bob_omb = 0x06,
    BlueShell = 0x07,
    Lightning = 0x08,
    Star = 0x09,
    GoldenMushroom = 0x0A,
    MegaMushroom = 0x0B,
    Blooper = 0x0C,
    POWBlock = 0x0D,
    ThunderCloud = 0x0E,
    BulletBill = 0x0F,
    TripleGreenShells = 0x10,
    TripleRedShells = 0x11,
    TripleBananas = 0x12,
    NoItem = 0x14,
};

enum class ItemObject {
    GreenShell = 0x00,
    RedShell = 0x01,
    Banana = 0x02,
    Mushroom = 0x03,
    Star = 0x04,
    BlueShell = 0x05,
    Lightning = 0x06,
    FakeItemBox = 0x07,
    MegaMushroom = 0x08,
    Bob_omb = 0x09,
    Blooper = 0x0A,
    POWBlock = 0x0B,
    GoldenMushroom = 0x0C,
    BulletBill = 0x0D,
    ThunderCloud = 0x0E,
    NoObject = 0x10,
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

static bool IsItemValid(ItemBox item)
{
    switch (item) {
    case ItemBox::GreenShell... ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanHoldItem(ItemBox item)
{
    switch (item) {
    case ItemBox::GreenShell... ItemBox::POWBlock:
    case ItemBox::BulletBill... ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool DoesItemHaveQuantity(ItemBox item)
{
    switch (item) {
    case ItemBox::TripleMushrooms:
    case ItemBox::TripleGreenShells:
    case ItemBox::TripleRedShells:
    case ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanUseItem(ItemBox item)
{
    if (item == ItemBox::NoItem) {
        return false;
    }

    u8 itemToUse = static_cast<u8>(item);

    return ItemBehaviour::s_useType[itemToUse] == ItemBehaviour::UseType::Use;
}

static bool CanThrowItem(ItemBox item)
{
    if (item == ItemBox::NoItem) {
        return false;
    }

    u8 itemToThrow = static_cast<u8>(item);

    return ItemBehaviour::s_useType[itemToThrow] ==
           ItemBehaviour::UseType::Throw;
}

static bool CanTrailItem(ItemBox item)
{
    if (item == ItemBox::NoItem) {
        return false;
    }

    u8 itemToTrail = static_cast<u8>(item);

    return !ItemBehaviour::s_hasUseFunction[itemToTrail];
}

static bool CanHitItemObject(ItemObject itemObject)
{
    switch (itemObject) {
    case ItemObject::GreenShell... ItemObject::BulletBill: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanItemObjectLockOn(ItemObject itemObject)
{
    switch (itemObject) {
    case ItemObject::RedShell:
    case ItemObject::BlueShell:
    case ItemObject::ThunderCloud: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool CanDropItemObject(ItemObject itemObject)
{
    switch (itemObject) {
    case ItemObject::GreenShell... ItemObject::BulletBill: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsHeldItemValidVS(ItemBox item)
{
    switch (item) {
    case ItemBox::GreenShell... ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidVS(ItemBox item)
{
    if (!IsHeldItemValidVS(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsHeldItemValidBB(ItemBox item)
{
    switch (item) {
    case ItemBox::GreenShell... ItemBox::Star:
    case ItemBox::MegaMushroom... ItemBox::Blooper:
    case ItemBox::TripleGreenShells... ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidBB(ItemBox item)
{
    if (!IsHeldItemValidBB(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsHeldItemValidCR(ItemBox item)
{
    switch (item) {
    case ItemBox::GreenShell... ItemBox::BlueShell:
    case ItemBox::Star... ItemBox::POWBlock:
    case ItemBox::TripleGreenShells... ItemBox::TripleBananas: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static bool IsTrailedItemValidCR(ItemBox item)
{
    if (!IsHeldItemValidCR(item)) {
        return false;
    }

    return CanTrailItem(item);
}

static bool IsItemObjectValid(ItemObject itemObject)
{
    switch (itemObject) {
    case ItemObject::GreenShell... ItemObject::ThunderCloud: {
        return true;
    }
    default: {
        return false;
    }
    }
}

static ItemBox ItemObjectToItemBox(ItemObject itemObject)
{
    switch (itemObject) {
    case ItemObject::GreenShell: {
        return ItemBox::GreenShell;
    }
    case ItemObject::RedShell: {
        return ItemBox::RedShell;
    }
    case ItemObject::Banana: {
        return ItemBox::Banana;
    }
    case ItemObject::Mushroom: {
        return ItemBox::Mushroom;
    }
    case ItemObject::Star: {
        return ItemBox::Star;
    }
    case ItemObject::BlueShell: {
        return ItemBox::BlueShell;
    }
    case ItemObject::Lightning: {
        return ItemBox::Lightning;
    }
    case ItemObject::FakeItemBox: {
        return ItemBox::FakeItemBox;
    }
    case ItemObject::MegaMushroom: {
        return ItemBox::MegaMushroom;
    }
    case ItemObject::Bob_omb: {
        return ItemBox::Bob_omb;
    }
    case ItemObject::Blooper: {
        return ItemBox::Blooper;
    }
    case ItemObject::POWBlock: {
        return ItemBox::POWBlock;
    }
    case ItemObject::GoldenMushroom: {
        return ItemBox::GoldenMushroom;
    }
    case ItemObject::BulletBill: {
        return ItemBox::BulletBill;
    }
    case ItemObject::ThunderCloud: {
        return ItemBox::ThunderCloud;
    }
    default: {
        return ItemBox::NoItem;
    }
    }
}

} // namespace wwfc::mkw::Item

#endif // RMC