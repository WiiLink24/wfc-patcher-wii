#pragma once

#include "wwfcLibC.hpp"
#include <wwfcUtil.h>

namespace wwfc::EGG
{

class Heap
{
public:
    virtual void vf_00() = 0;
    virtual void vf_04() = 0;
    virtual void vf_08() = 0;
    virtual void vf_0C() = 0;
    virtual void vf_10() = 0;
    virtual void* alloc(std::size_t size, int alignment) = 0;
    virtual void free(void* block) = 0;
    virtual void vf_1C() = 0;
    virtual void vf_20() = 0;
    virtual void vf_24() = 0;
    virtual void vf_28() = 0;

    static void* Alloc(std::size_t size, int alignment, Heap* heap)
    {
#if RMC
        LONGCALL void* Alloc(std::size_t size, int alignment, Heap* heap)
            AT(RMCXD_PORT(0x80229814, 0x80229490, 0x80229734, 0x80229B88));

        return Alloc(size, alignment, heap);
#endif
        return nullptr;
    }

    static void Free(void* block, Heap* heap)
    {
#if RMC
        LONGCALL void Free(void* block, Heap* heap)
            AT(RMCXD_PORT(0x80229B84, 0x80229800, 0x80229AA4, 0x80229EF8));

        Free(block, heap);
#endif
    }

private:
    /* 0x04 */ u8 _04[0x38 - 0x04];
};

static_assert(sizeof(Heap) == 0x38);

} // namespace wwfc::EGG

inline void*
operator new(wwfc::std::size_t size, wwfc::EGG::Heap* heap, int alignment)
{
    return wwfc::EGG::Heap::Alloc(size, alignment, heap);
}
