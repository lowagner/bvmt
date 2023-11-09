#pragma once

#include "error.h"
#include "optional.h" 
#include "pointer.h" 
#include "types.h" 

#include <iterator>

BVMT

#define ITERATOR_CONSTRUCTOR_TEMPLATE_BASE(x, preLoopLogic, type, TypeVar, loopLogic) \
    x(iterator<type> Iterator) \
    {   preLoopLogic; \
        for (type TypeVar : Iterator) \
        {   loopLogic; \
        } \
    }

#define ITERATOR_CONSTRUCTOR_TEMPLATES(x, preLoopLogic, type, TypeVar, loopLogic) \
    ITERATOR_CONSTRUCTOR_TEMPLATE_BASE(x, preLoopLogic, type, TypeVar, loopLogic) \
    ITERATOR_CONSTRUCTOR_TEMPLATE_BASE(x, preLoopLogic, type &, TypeVar, loopLogic) \
    ITERATOR_CONSTRUCTOR_TEMPLATE_BASE(x, preLoopLogic, const type &, TypeVar, loopLogic)

#define ITERATOR_ASSIGNMENT_TEMPLATE_BASE(x, logicBeforeAssignment, type, TypeVar, loopLogic) \
    x &operator = (iterator<type> &&Iterator) \
    {   logicBeforeAssignment; \
        for (type TypeVar : Iterator) \
        {   loopLogic; \
        } \
        return This; \
    }

#define ITERATOR_ASSIGNMENT_TEMPLATES(x, logicBeforeAssignment, type, TypeVar, loopLogic) \
    ITERATOR_ASSIGNMENT_TEMPLATE_BASE(x, logicBeforeAssignment, type, TypeVar, loopLogic) \
    ITERATOR_ASSIGNMENT_TEMPLATE_BASE(x, logicBeforeAssignment, type &, TypeVar, loopLogic) \
    ITERATOR_ASSIGNMENT_TEMPLATE_BASE(x, logicBeforeAssignment, const type &, TypeVar, loopLogic)

#define ITERATOR_PLUS_EQUAL_TEMPLATE_BASE(x, preLogic, type, TypeVar, loopLogic) \
    x &operator += (iterator<type> &&Iterator) \
    {   preLogic; \
        for (type TypeVar : Iterator) \
        {   loopLogic; \
        } \
        return This; \
    }

#define ITERATOR_PLUS_EQUAL_TEMPLATES(x, preLogic, type, TypeVar, loopLogic) \
    ITERATOR_PLUS_EQUAL_TEMPLATE_BASE(x, preLogic, type, TypeVar, loopLogic) \
    ITERATOR_PLUS_EQUAL_TEMPLATE_BASE(x, preLogic, type &, TypeVar, loopLogic) \
    ITERATOR_PLUS_EQUAL_TEMPLATE_BASE(x, preLogic, const type &, TypeVar, loopLogic)

#define KERNEL_TO_ITERATOR_LOGIC(kernelType, iteratorValue, args, Args) \
{   return iterator<iteratorValue> \
    (   pointer<iteratorKernel<iteratorValue>> \
        (   new kernelType Args, \
            [](iteratorKernel<iteratorValue> *Kernel) \
            {   /* need to cast to the real type for the destructor to work properly: */ \
                delete (kernelType *)Kernel; \
            } \
        ) \
    ); \
}

#define KERNEL_TO_ITERATOR_H(kernelType, iteratorValue, args, Args) \
    static iterator<iteratorValue> toIterator args;

#define KERNEL_TO_ITERATOR_CC(kernelType, iteratorValue, args, Args) \
    iterator<iteratorValue> kernelType::toIterator args \
    KERNEL_TO_ITERATOR_LOGIC(SAFE((kernelType)), iteratorValue, args, Args)

#define KERNEL_TO_ITERATOR(kernelType, iteratorValue, args, Args) \
    static iterator<iteratorValue> toIterator args \
    KERNEL_TO_ITERATOR_LOGIC(SAFE((kernelType)), iteratorValue, args, Args)

template <class t>
class iterator;

/*
C++ doesn't do generics (templates) with polymorphism (virtual/overloading methods)
so a generic function that needs overloads needs functions to be passed into it to define those methods.

ideally, it doesn't even care about the `state`, as that's an implementation detail.
callers/users should only need to know about what is being iterated over.  what we do...
    *   create a `new` state struct/info variable (iteratorKernel or an extending/child class).
    *   have a cleanup function which frees the pointer when the iterator is descoped.
*/
template <class t>
struct iteratorKernel
{   using basePtr = iteratorKernel<t> *;
    // pass in the basePtr type, but you should cast it to whatever your iteratorKernel child-class is.
    fn<optional<t>(basePtr)> next;
    // TODO: add support for these, as well as for checking if they are present inside iterator.
    //fn<const t(basePtr)> peak;
    //fn<t(basePtr)> previous;
    //fn<valueType(basePtr)> take;
    //fn<index(basePtr)> remainingCount;
    // TODO: double check that we need to do the `pointer` route
};

// TODO: double check, do we want to invert the control flow?  e.g., instead of the site calling
// the iterator for the next value, do we want the iterator/container calling an iterand function
// for each value?
// e.g., currently: `iterator<int> Iterator = ...; optional<int> Next = Iterator.next(); ...`
// would switch to: `iterator<int> Iterator; Iterator.iterate([](int Value) { LOG(Value); }`
// can we do filterMap functionality here?  `Iterator.map([](int Value) { return string::of(Value); }`

template <class t>
struct iteratorRange
{   // Start at this value.
    t Start = 0;
    // Iterate but stop before this value.
    t EndBefore = 0;
    // TODO: step
};


#define CONTAINER_ITERATOR(container) \
    template <class containerElementPointer> \
    class containerStlIterator \
    {   containerElementPointer Pointer; \
    public: \
        using value_type = decltype(Pointer.element()); \
        using iterator_category = std::forward_iterator_tag; \
        \
        containerStlIterator(containerElementPointer _Pointer) \
        :   Pointer(_Pointer) \
        {} \
        \
        value_type operator * () const \
        {   return Pointer.element(); \
        } \
        \
        containerStlIterator &operator ++ () /* Pre-increment */ \
        {   Pointer.next(); \
            return This; \
        } \
        /* Assume that we only care about comparing to the end() of the iterator. */ \
        bool operator == (sentinel) \
        {   return Pointer == Null; \
        } \
    }; \
    \
    containerStlIterator<constElementPointer> begin() const \
    {   return containerStlIterator<constElementPointer>(first()); \
    } \
    sentinel end() const { return sentinel{}; } \
    \
    template <class containerPointer, class containerElementPointer, class containerElement>  \
    class containerIteratorKernel : public iteratorKernel<containerElement> \
    {   /* TODO: make this a pointer<> so that we can take it in (and destroy it if need be) \
           when the container does not outlive the iterator. */ \
        containerPointer Container; \
        containerElementPointer Pointer; \
        bool HasStarted = False; \
        \
        containerIteratorKernel(containerPointer _Container) \
        :   iteratorKernel<containerElement> \
            ({  .next = [](iteratorKernel<containerElement> *BaseKernelSelf) \
                {   CAST_DEFINE \
                    (   SAFE((containerIteratorKernel<containerPointer, containerElementPointer, containerElement>)) *, \
                        Self, \
                        BaseKernelSelf \
                    ); \
                    if (!Self->HasStarted) \
                    {   Self->HasStarted = True; \
                        Self->Pointer = Self->Container->first(); \
                    } \
                    else if (Self->Pointer != Null) \
                    {   Self->Pointer.next(); \
                    } \
                    if (Self->Pointer == Null) \
                    {   return optional<containerElement>(Null); \
                    } \
                    return optional(Self->Pointer.element()); \
                }, \
            }), \
            Container(_Container) \
        {} \
        \
    public: \
        KERNEL_TO_ITERATOR \
        (   containerIteratorKernel<containerPointer AND containerElementPointer AND containerElement>, \
            containerElement, \
            (containerPointer _Container), \
            (_Container) \
        ); \
    }; \
    \
    iterator<constElement> elements() const & \
    {   return containerIteratorKernel<const container *, constElementPointer, constElement> \
                ::toIterator(this); \
    }

#define MUTABLE_CONTAINER_ITERATOR(container) \
    containerStlIterator<elementPointer> begin() \
    {   return containerStlIterator<elementPointer>(first()); \
    } \
    sentinel end() { return sentinel{}; } \
    \
    iterator<element> elements() & \
    {   return containerIteratorKernel<container *, elementPointer, element>::toIterator(this); \
    }



namespace detail
{   // TODO: maybe create a wrapper around standard STL iterators, e.g.
    // template <class t, class stlItr>
    // so it's easy to port more stl iterators.  or not...

    // Buffers a `next` value from the iterator.
    template <class t>
    class stlIterator 
    {   iterator<t> *Iterator;
        optional<t> CurrentValue;
    public:
        using value_type = typename std::remove_reference<t>::type;
        using reference = value_type &;
        using pointer = value_type *;
        // TODO: maybe make an optional template argument, or throw errors if operations unsupported:
        using iterator_category = std::forward_iterator_tag;
        // TODO: do not use differences of the iterator, these details should be ignored:
        //using difference_type = NOT std::ptrdiff_t;

        stlIterator(iterator<t> *_Iterator, bool AtEnd)
        :   Iterator(_Iterator)
        {   if (!AtEnd)
            {   CurrentValue = Iterator->next();
            }
        }

        // Gets the current value:
        reference operator * () const
        {   ASSERT(CurrentValue != Null);
            return (reference)*CurrentValue;
        }

        stlIterator &operator ++ ()
        {   // pre-increment
            CurrentValue = Iterator->next();
            return *this;
        }

        friend bool operator == (const stlIterator &Lhs, const stlIterator &Rhs)
        {   // Assume that we only care about comparing to the end() of the iterator.
            return Lhs.Iterator == Rhs.Iterator
                    && (Lhs.CurrentValue == Null) == (Rhs.CurrentValue == Null);
        }

        friend bool operator != (const stlIterator &Lhs, const stlIterator &Rhs)
        {   return !(Lhs == Rhs);
        }
    };

    template <class t>
    class iteratorRangeKernel : public iteratorKernel<t>
    {   t Index; // current index while iterating.
        t EndBefore; // don't return this index when iterating.

        iteratorRangeKernel(iteratorRange<t> Range)
        :   iteratorKernel<t>
            ({  .next = [](iteratorKernel<t> *BaseKernelSelf)
                {   CAST_DEFINE
                    (   iteratorRangeKernel<t> *,
                        Self,
                        BaseKernelSelf
                    );
                    if (Self->Index + t(1) < Self->EndBefore)
                    {   return optional<t>(++(Self->Index));
                    }
                    return optional<t>();
                },
            }),
            Index(Range.Start - t(1)),
            EndBefore(Range.EndBefore)
        {}
    public:
        static pointer<iteratorKernel<t>> destructible(iteratorRange<t> Range)
        {   return pointer<iteratorKernel<t>>
            (   new iteratorRangeKernel<t>(Range),
                [](iteratorKernel<t> *Kernel)
                {   // need to cast to the real type for the destructor to work properly:
                    delete (iteratorRangeKernel<t> *)Kernel;
                }
            );
        }
    };

    // maps type from t -> u, using refT in mapped function arg.
    template <class t, class refT, class u>
    class iteratorKernelFilterMap : public iteratorKernel<u>
    {   pointer<iteratorKernel<t>> ParentKernel;
        fn<optional<u>(refT)> mapTToU;

        iteratorKernelFilterMap
        (   pointer<iteratorKernel<t>> _ParentKernel,
            fn<optional<u>(refT)> _mapTToU
        )
        :   iteratorKernel<u>
            ({  .next = [](iteratorKernel<u> *BaseKernelSelf)
                {   CAST_DEFINE
                    (   SAFE((iteratorKernelFilterMap<t, refT, u>)) *,
                        Self,
                        BaseKernelSelf
                    );
                    for
                    (   optional<t> T = Self->ParentKernel->next(&*Self->ParentKernel);
                        T != Null;
                        T = Self->ParentKernel->next(&*Self->ParentKernel)
                    )
                    {   optional<u> U = Self->mapTToU(*T);
                        if (U != Null)
                            return U;
                    }
                    return optional<u>();
                }
            }),
            ParentKernel(std::move(_ParentKernel)),
            mapTToU(_mapTToU)
        {}
    public:
        static pointer<iteratorKernel<u>> destructible
        (   pointer<iteratorKernel<t>> _ParentKernel,
            fn<optional<u>(refT)> _mapTToU
        )
        {   return pointer<iteratorKernel<u>>
            (   new iteratorKernelFilterMap<t, refT, u>(std::move(_ParentKernel), _mapTToU),
                [](iteratorKernel<u> *Kernel)
                {   // need to cast to the real type for the destructor to work properly:
                    delete (iteratorKernelFilterMap<t, refT, u> *)Kernel;
                }
            );
        }
    };
}

template <class t>
class iterator
{   pointer<iteratorKernel<t>> Kernel;
public:
    iterator(pointer<iteratorKernel<t>> _Kernel) : Kernel(std::move(_Kernel)) {}

    optional<t> next()
    {   ASSERT(Kernel->next != Null);
        return Kernel->next(&*Kernel);
    }

    bool checkAny(fn<bool(const t &)> hasCondition)
    {   for (const t & Entry : This)
        {   if (hasCondition(Entry))
            {   return true;
            }
        }
        return false;
    }

    // Create a new, child iterator which maps over some elements based on the values of
    // the parent (this).  The parent (this) MUST outlive the child (returned) iterator.
    template <class u>
    iterator<u> iterate
    (   // this mapping function does not require that `t` is immutable, since we are
        // iterating through and if it's changed, it will disappear anyways.
        // note that `t` can of course be a `const` type under the hood.
        fn<optional<u>(t &)> mapTToU
    )   &
    {   return iterator<u>
        (   detail::iteratorKernelFilterMap<t, t &, u>::destructible
            (   Kernel.reference(),
                mapTToU
            )
        );
    }

    // An overload of iterate for when the parent (this) will be descoped
    // before the child (returned iterator).
    template <class u>
    iterator<u> iterate
    (   fn<optional<u>(t &)> mapTToU
    )   &&
    {   return iterator<u>
        (   detail::iteratorKernelFilterMap<t, t &, u>::destructible
            (   Kernel.abdicate(),
                mapTToU
            )
        );
    }

    // TODO: add a `join` method on string.
    // Useful so you don't have to remember whether it's `",".join(Iterator)` or `Iterator.join(",")`
    // TODO: we can probably return a string here, and do string(Delimiter).join(...).
    // i can't think of another good use-case for joining into a non-string.
    template <class u>
    inline u join(u Delimiter) &&
    {   return Delimiter.join(std::move(This));
    }

    // TODO: add support for creating a new iterator via a `take` function.
    // This is probably for container classes which can be removed/erased.
    // E.g., `iterator<t> container<t>::take(fn<bool(const t &)> where)`.
    // Probably want to do this as e.g., `array/map/set` inheriting from `container<t>`.

    using iterator_type = detail::stlIterator<t>;

    // Beware of using these STL iterators for standard C++ things.
    // These are currently designed only for making for-loops with containers easy:
    iterator_type begin() { return iterator_type(this, /*AtEnd =*/ False); }
    iterator_type end() { return iterator_type(this, /*AtEnd =*/ True); }

    // TODO: add support for iterator + iterator and iterator += iterator.
    // create a special multiIteratorKernel which has an array<pointer<iteratorKernel>>
    // to iterate through.

    // Returns an iterator from 0 to Count-1.
    static iterator<t> range(t Count)
    {   return iterator<t>
        (   detail::iteratorRangeKernel<t>::destructible(iteratorRange<t>({.EndBefore = Count}))
        );
    }

    static iterator<t> range(iteratorRange<t> Range)
    {   return iterator<t>(detail::iteratorRangeKernel<t>::destructible(Range));
    }
};

#ifndef NDEBUG
namespace test
{   iterator<noisy &> noisyIterator(int Count);
}
#endif

TMVB
