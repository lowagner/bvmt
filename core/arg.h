#pragma once

#include "types.h"

BVMT

#define ARG_ACCESS_OPERATORS(modifier) \
    inline modifier t *operator ->() modifier \
    {   return &T; \
    } \
    inline modifier t &operator * () modifier \
    {   return T; \
    }

#define ARG_OUT_OPERATORS(extraLogic) \
    inline t &operator = (t &&NewT) \
    {   T = std::move(NewT); \
        extraLogic; \
        return T; \
    } \
    inline t &operator = (const t &CopyT) \
    {   T = CopyT; \
        extraLogic; \
        return T; \
    }

namespace arg
{   // Class that represents a pure input, i.e., no changes are allowed
    // to be made to this argument that would escape the function scope.
    // TODO: maybe do a variant here, either a `const t &T` or a `T` for copy-on-write semantics
    template <class t>
    class in
    {   t &T;
    public:
        in(t &_T) : T(_T) {}
        in(t &&_T) = delete;

        // TODO: for some reason this doesn't work
        // for convenience, convert between const `in` and non-const `in`.
        // in(in<typename type<t>::switchConst> In) : T(In.T) {}

        ARG_ACCESS_OPERATORS(const)
    };
    
    template <class t>
    class out;

    // Class that represents a function argument that should only be written to,
    // but if written to and later desired to be reset to what was passed in,
    // this class will allow the argument to be reset.  The written (or reset)
    // value escapes the scope of the function, thus, it is output like.
    template <class t>
    class resettableOut
    {   t &T;
        t OriginalValue;
        bool ResetOnDescope = True;

        friend out<t>;
    public:
        resettableOut(t &_T) : T(_T), OriginalValue(std::move(_T)) {}
        resettableOut(t &&_T) = delete;

        UNCOPYABLE_CLASS(resettableOut)
        UNMOVABLE_CLASS(resettableOut)

        ~resettableOut()
        {   if (ResetOnDescope)
            {   T = std::move(OriginalValue);
            }
        }

        inline void doNotResetOnDescope()
        {   ResetOnDescope = False;
        }

        // NOTE: You can technically get a reference to the value T,
        // then descope this resettableOut, then see what the original
        // value was.  This is against the spirit of resettableOut,
        // like un-`const`-ing a `const` value, but there are other
        // ways to get the reference (e.g., by casting the first bytes
        // of memory of this class into a pointer to `t`), so we can't
        // (and won't) stop all misuses of this class.
        // NOTE: Setting the value on *this* resettable will toggle no resetting.
        // WARNING! Setting the value on the original arg::out (which created this
        // arg::resettableOut, if created from Out::resettable()) will not set no-reset.
        ARG_OUT_OPERATORS(doNotResetOnDescope());
    };

    // Class that represents a function argument that should only be written to,
    // The written value escapes the scope of the function, so it is more like
    // an output argument (return value) than an input argument.
    // NOTE! Prefer using `bvmt::determining` instead of `bvmt::arg::out` for
    // the greater grammatical flow it provides.
    template <class t>
    class out
    {   t &T;
    public:
        out(resettableOut<t> Resettable) : T(Resettable.T) {}
        explicit out(t &_T) : T(_T) {}
        out(t &&_T) = delete;

        resettableOut<t> resettable()
        {   return resettableOut(T);
        }

        ARG_OUT_OPERATORS({})
    };

    // Class that represents a function argument that can be read or written.
    // Whatever value it is escapes the scope of the function, so it can be
    // both like an input argument and an output argument (return value).
    // NOTE! Prefer using `bvmt::suggesting` instead of `bvmt::arg::io` for
    // the greater grammatical flow it provides.
    template <class t>
    class io
    {   t &T;
    public:
        explicit io(t &_T) : T(_T) {}
        io(t &&_T) = delete;

        ARG_ACCESS_OPERATORS(const)
        ARG_ACCESS_OPERATORS()
        ARG_OUT_OPERATORS({})
    };
}

template <class t>
using defaultTo = arg::io<t>;

template <class t>
using determining = arg::out<t>;

template <class t>
using updating = arg::io<t>;

template <class t>
using swapping = arg::io<t>;

template <class t>
using suggesting = arg::io<t>;

// This indicates that the result will be no less than the passed-in value,
// but this is not guaranteed by the underlying arg::io class; this must be
// guaranteed by the class/function which is using the suggestingAtLeast argument.
template <class t>
using suggestingAtLeast = arg::io<t>;

TMVB
