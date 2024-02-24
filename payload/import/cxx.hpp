#pragma once

#include "import/egg/heap.hpp"

#if RMC

void* operator new(size_t size, EGG::Heap* heap, int alignment)
{
    return EGG::Heap::Alloc(size, alignment, heap);
}

void operator delete(void* block)
{
    EGG::Heap::Free(block, nullptr);
}

#endif
