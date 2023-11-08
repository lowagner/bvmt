#pragma once

#include "arg.h"
#include "error.h"
#include "types.h"

#include <stdlib.h> // malloc, realloc, oh yeah i'm weird

BVMT

DEBUG_ONLY(static bool MemoryDebug = False);

#define LOG_MEMORY(X) DEBUG_ONLY \
(   if (TestOnly || MemoryDebug) std::cout << X \
)

namespace memory
{   // Allocates a number of elements without initialization.
    // Will update the Number passed in if the memory allocator returns more space than you asked for.
    // Will return Null if we can't allocate that much memory.
    template <class t>
    t *allocate(suggestingAtLeast<index> Number)
    {   size_t Memory = *Number * sizeof(t);
        if (*Number < 0 || Memory < (size_t)*Number)
        {   // overflow or something else weird
            LOG_MEMORY("allocate<" << sizeof(t) << "x>(" << *Number << ")|Null|");
            return Null;
        }
        t *Result = (t *)malloc(Memory);
        LOG_MEMORY("allocate<" << sizeof(t) << "x>(" << *Number << ")|"  << (int *)Result << "|");
        return Result;
    }

    template <class t>
    inline t *allocate(index Number)
    {   return allocate<t>(suggestingAtLeast(Number));
    }

    template <class t>
    inline void defaultConstruct(t *UninitializedMemory)
    {   new (UninitializedMemory) t();
    }

    template <class t>
    inline void copyConstruct(t *UninitializedMemory, const t &ToCopy)
    {   new (UninitializedMemory) t(ToCopy);
    }

    template <class t>
    inline void moveConstruct(t *UninitializedMemory, t &&ToMove)
    {   new (UninitializedMemory) t(std::move(ToMove));
    }

    template <class t>
    inline void deconstruct(t *InitializedMemory)
    {   InitializedMemory->~t();
    }

    // Reallocates a number of elements without initialization of new elements.
    // Will update the Number passed in if the memory allocator returns more space than you asked for.
    // Will return Null if we can't allocate that much memory.
    template <class t>
    t *reallocate(t *CurrentPointer, suggestingAtLeast<index> Number)
    {   size_t Memory = *Number * sizeof(t);
        if (*Number < 0 || Memory < (size_t)*Number)
        {   // overflow or something else weird
            LOG_MEMORY
            (   "reallocate<" << sizeof(t) << "x>(" << (int *)CurrentPointer << ", " << *Number << ")"
                        "|Null|"
            );
            return Null;
        }
        t *Result = (t *)realloc(CurrentPointer, Memory);
        LOG_MEMORY
        (   "reallocate<" << sizeof(t) << "x>(" << (int *)CurrentPointer << ", " << *Number << ")"
                    "|" << (int *)Result << "|"
        );
        return Result;
    }

    template <class t>
    inline t *reallocate(t *CurrentPointer, index Number)
    {   return reallocate(CurrentPointer, suggestingAtLeast(Number));
    }

    template <class t>
    inline void deallocate(t *Pointer)
    {   LOG_MEMORY("deallocate(" << (int *)Pointer << ")");
        free(Pointer);
    }
}

TMVB

#endif
