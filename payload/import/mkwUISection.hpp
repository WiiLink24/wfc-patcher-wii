#pragma once

#include "mkwUIPageId.hpp"
#include <array>
#include <wwfcCommon.h>

namespace mkw::UI
{

#if RMC

class Page;

class Section
{
public:
    template <typename T>
    T* page(PageId pageId)
    {
        return reinterpret_cast<T*>(m_page[static_cast<s32>(pageId)]);
    }

private:
    /* 0x000 */ u8 _000[0x008 - 0x000];
    /* 0x008 */ std::array<Page*, 211> m_page;
    /* 0x354 */ u8 _354[0x408 - 0x354];
};

static_assert(sizeof(Section) == 0x408);

#endif

} // namespace mkw::UI
