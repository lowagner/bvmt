#pragma once

#include "error.h" 
#include "memory.h"
#include "pointer.h" 
#include "types.h" 

BVMT

namespace detail
{   template <class t, typename Enable = void>
    struct optionalSpecification
    {   
    };

    // reference overload, to save memory and not store a value type (see non-reference overload)
    template <class t> 
    struct optionalSpecification<t, std::enable_if_t<type<t>::IsCppPointer>>
    {   using pointingType = typename type<t>::pointingAt;

        pointingType *Pointer = Null;

        optionalSpecification() {}

        optionalSpecification(pointingType &Reference) : Pointer(&Reference) {}

        optionalSpecification(pointingType *_CPointer) : Pointer(_CPointer) {}

        ~optionalSpecification()
        {   reset();
        }

        inline void reset()
        {   Pointer = Null;
        }

        MOVABLE_TEMPLATE
        (   optionalSpecification, Moved,
            // assignment only logic:
            reset(),
            // both construct and assign:
            std::swap(Pointer, Moved.Pointer);
        );

        COPYABLE_TEMPLATE
        (   optionalSpecification, ToCopy,
            // assignment only logic:
            reset(),
            // both construct and assign:
            Pointer = ToCopy.Pointer;
        );
    };

    // TODO: maybe? or maybe not; these have bad copy semantics.
    // template <class t> 
    // struct optionalSpecification<t, std::enable_if_t<type<t>::IsPointer>>

    // non-reference overload, can be a pointer or a value.
    template <class t>
    struct optionalSpecification<t, std::enable_if_t<!type<t>::IsPointerLike>>
    {   using valueType = t;

        union
        {   // This union is used to ensure that `Value` is not default initialized.
            u8 IgnoreMe;
            valueType Value;
        };
        bool HasValue = False;

        optionalSpecification() {}

        ~optionalSpecification()
        {   reset();
        }

        inline void reset()
        {   if (HasValue)
            {   memory::deconstruct(&Value);
                HasValue = False;
            }
        }

        COPYABLE_TEMPLATE
        (   optionalSpecification, ToCopy,
            // assignment only logic:
            reset(),
            // both construct and assign:
            if (ToCopy.HasValue)
            {   HasValue = True;
                memory::copyConstruct(&Value, ToCopy.Value);
            }
        );

        MOVABLE_TEMPLATE
        (   optionalSpecification, Moved,
            // assignment only logic:
            reset(),
            // both construct and assign:
            if (Moved.HasValue)
            {   HasValue = True;
                memory::moveConstruct(&Value, std::move(Moved.Value));
                Moved.reset();
            }
        );
    };
}

// a class which holds an instance of some type, or maybe not.
// note that we also allow pointers to be held internally as well.
template <class t>
class optional
{   using internalType = typename type<t>::pointingAt;
    detail::optionalSpecification<t> Specification;
public:
    inline void reset()
    {   Specification.reset();
    }

    optional() {}
    optional(null) {}

    optional(t T)
    {   if IS_POINTER_LIKE(t)
        {   Specification = detail::optionalSpecification<t>(T);
        }
        else
        {   Specification.HasValue = True;
            memory::moveConstruct(&Specification.Value, std::move(T));
        }
    }

    ENABLE_FOR_VALUE_TYPES(t)
    inline void value(internalType &&T)
    {   reset();
        Specification.HasValue = True;
        memory::moveConstruct(&Specification.Value, std::move(T));
    }

    ENABLE_FOR_VALUE_TYPES(t)
    optional &operator = (const internalType &T)
    {   t Copy = T;
        value(std::move(Copy));
        return This;
    }

    ENABLE_FOR_VALUE_TYPES(t)
    optional &operator = (internalType &&T)
    {   value(std::move(T));
        return This;
    }

    // throws if this optional is Null:
    inline internalType *operator -> ()
    {   return (internalType *)(((const optional *)this)->operator -> ());
    }

    // throws if this optional is Null:
    inline const internalType *operator -> () const
    {   if IS_POINTER_LIKE(t)
        {   if (Specification.Pointer != Null)
            {   return Specification.Pointer;
            }
        }
        else if (Specification.HasValue)
        {   return &(Specification.Value);
        }
        throw error("optional is Null", AT);
    }

    // throws if this optional is Null:
    inline internalType &operator * ()
    {   return *(operator -> ());
    }

    // throws if this optional is Null:
    inline const internalType &operator * () const
    {   return *(operator -> ());
    }

    // TODO: `inline const t &value() const` which references the value or pointer value,
    // or returns a default.
   
    // TODO: see optional.cc bit for making this ok to take from as long as nonNull, even if pointer
    ENABLE_FOR_VALUE_TYPES(t)
    inline t takeValue()
    {   if (This == Null)
        {   throw error("optional is Null", AT);
        }
        t ReturnValue = std::move(Specification.Value);
        // Need to make sure this optional gets put back to the default state,
        // so delete the old memory at Value (if any, probably shouldn't be)
        // and move to the null pointer state:
        reset();
        return ReturnValue;
    }

    bool operator == (const optional &Other) const
    {   if (This == Null)
        {   return Other == Null;
        }
        // This != Null
        if (Other == Null)
        {   return False;
        }
        return *This == *Other;
    }
    
    // Should be used only for checking against Null.
    inline bool operator != (null) const
    {   if IS_POINTER_LIKE(t)
        {   return Specification.Pointer != Null;
        }
        else
        {   return Specification.HasValue;
        }
    }

    // Should be used only for checking against Null.
    inline bool operator == (null) const
    {   return !(operator != (Null));
    }
};

template <class t>
std::ostream &operator << (std::ostream &Out, const optional<t> &Optional)
{   Out << "optional(";
    if (Optional != Null)
    {   if IS_POINTER_LIKE(t)
        {   Out << "Pointer: " << *Optional;
        }
        else
        {   Out << "Value: " << *Optional;
        }
    }
    return Out << ")";
}

TMVB
