#pragma once

#include "error.h"
#include "types.h"

BVMT

template <class t>
class pointer
{   // A class which has some specific logic to destroy the held pointer or not.
    // if onDestroy is not defined, this acts as a reference class (i.e., the pointer is not freed).
    // to make this like a unique_ptr class, onDestroy should free the passed in pointer.
    // E.g., see class static functions `pointer::reference(Ptr)` and `pointer::deleteOnDescope(Ptr)`.
    t *Ptr;
    fn<void(t *)> onDestroy;

    pointer(t *_Ptr)
    :   Ptr(_Ptr), onDestroy(Null)
    {}

    friend optional<t>;
public:
    pointer()
    :   Ptr(Null), onDestroy(Null)
    {}

    UNCOPYABLE_CLASS(pointer)

    MOVABLE_TEMPLATE
    (   pointer, Pointer,
        // For move assignment, make sure this Ptr is handled correctly first:
        reset(),
        // Do the rest for both assignment and construction:
        // Don't swap Pointer/This.Ptr since Ptr can be non-Null with move-construction.
        Ptr = Pointer.Ptr;
        Pointer.Ptr = Null;
        onDestroy = Pointer.onDestroy;
        Pointer.onDestroy = Null;
    )

    // Reference-like version of the class, no destroy:
    static pointer<t> reference(t *_Ptr)
    {   return pointer(_Ptr);
    }

    // TODO: add a static arg-forwarding emplace-back type constructor.

    // A unique_ptr-like version of this class; it will just call `delete` on the passed-in pointer
    // when this instance is descoped.
    static pointer<t> deleteOnDescope(t *_Ptr)
    {   return pointer(_Ptr, [](t * T) { delete T; });
    }

    pointer(t *_Ptr, fn<void(t *)> _onDestroy)
    :   Ptr(_Ptr), onDestroy(_onDestroy)
    {   ASSERT(onDestroy != Null);
    }

    ~pointer() { reset(); }

    void reset()
    {   fn<void(t *)> onDestroyOnce;
        // Call the destructor lambda only after we've reset this pointer,
        // so that we don't keep the pointer here in case of failure.
        std::swap(onDestroy, onDestroyOnce);
        if (Ptr != Null)
        {   t *PtrOnce = Null;
            std::swap(PtrOnce, Ptr);
            if (onDestroyOnce)
                onDestroyOnce(PtrOnce);
        }
    }

    // Returns a pointer that doesn't have a destructor.
    // This should outlive the returned pointer, or things will go bad.
    pointer<t> reference()
    {   return pointer(Ptr);
    }

    bool isReference() const
    {   return onDestroy == Null;
    }

    bool isOwned() const
    {   return onDestroy != Null;
    }

    // Returns a copy of this destructible, and removes the destroy callback from this destructible.
    // The new pointer will be responsible for cleaning up (if this instance originally was),
    // and this pointer will maintain a reference to the instance.
    // Use std::move on this instance if you want to remove this' reference as well.
    pointer<t> abdicate()
    {   pointer<t> Result(Ptr, std::move(onDestroy));
        onDestroy = Null;
        return Result;
    }

    template <class u = t> requires (!type<u>::IsVoid)
    u *operator -> ()
    {   if (Ptr == Null)
            throw error("Cannot access fields of null pointer.", AT);
        return Ptr;
    }
    
    template <class u = t> requires (!type<u>::IsVoid)
    const u *operator -> () const
    {   if (Ptr == Null)
            throw error("Cannot access fields of null pointer.", AT);
        return Ptr;
    }

    template <class u = t> requires (!type<u>::IsVoid)
    u &operator * ()
    {   if (Ptr == Null)
            throw error("Cannot access fields of null pointer.", AT);
        return *Ptr;
    }

    template <class u = t> requires (!type<u>::IsVoid)
    const u &operator * () const
    {   if (Ptr == Null)
            throw error("Cannot access fields of null pointer.", AT);
        return *Ptr;
    }

    // Should be used only for checking against Null.
    bool operator == (null) const
    {   return Ptr == Null;
    }

    // Should be used only for checking against Null.
    bool operator != (null) const
    {   return Ptr != Null;
    }
};

TMVB
