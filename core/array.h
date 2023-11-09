#pragma once

#include "arg.h"
#include "error.h"
#include "iterator.h"
#include "optional.h"
#include "pointer.h"
#include "types.h"

#include <algorithm>    // std::reverse, std::sort 
#include <vector>

BVMT

template <class t>
class array;

extern const char *const ArrayViewEmptyMsg;

template <class t>
class arrayView
{   // A view of an array.  Note that any changes to the original array's size
    // should be assumed to invalidate the pointers here and break this view.
    t *Start;
    t *End;
public:
    arrayView(t *_Start, t *_End) : Start(_Start), End(_End) {}

    inline index count() const
    {   return (index)(End - Start);
    }

    inline bool empty() const
    {   return Start >= End;
    }

    // Returns a reference to the first element in the array,
    // and moves this arrayView pointer up.
    // NOTE! This does *not* change the size of the original array.
    inline t &shiftView()
    {   if (empty())
        {   throw error(ArrayViewEmptyMsg, AT);
        }
        t *OldStart = Start++;
        return *OldStart;
    }

    arrayView &shiftStart(index Amount)
    {   Start += Amount;
        return This;
    }
    
    // Returns a reference to the last element in the array,
    // and shrinks the size of this arrayView.
    // NOTE! This does *not* change the size of the original array.
    inline t &popView()
    {   if (empty())
        {   throw error(ArrayViewEmptyMsg, AT);
        }
        return *--End;
    }
};

#define ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index) \
    if (Index < 0) \
    {   /* Put Index in [0, count() - 1], hopefully: */ \
        Index += count(); \
        if (Index < 0) \
            throw error(ArrayTooNegativeIndexErrorMsg, AT); \
    }    

extern const char *const ArrayTooNegativeIndexErrorMsg;
extern const char *const ArrayPoppingErrorMsg;
extern const char *const ArrayInvalidArgumentErrorMsg;
extern const char *const ArrayFixedCountErrorMsg;

template <class value>
struct arrayElement
{   const index Index;
    value Value;

    using noRefValueType = typename std::remove_reference<value>::type;

    arrayElement(index I, value V)
    :   Index(I), Value(V)
    {}

    template <class newValue>
    arrayElement(const arrayElement<newValue> &ArrayElement)
    :   Index(ArrayElement.Index), Value(ArrayElement.Value)
    {}
};

template <class value>
std::ostream &operator << (std::ostream &Out, const arrayElement<value> &Element)
{   return Out << "arrayElement(" << Element.Index << ", " << Element.Value << ")";
}

namespace arrayDetail
{   template <class arrayPointer, class value, class el>
    class elementPointer
    {   arrayPointer Array = Null;
        index Index = -1;

        template <class v>
        friend class bvmt::array;

        elementPointer(arrayPointer _Array, index _Index)
        :   Array(_Array), Index(_Index)
        {}
    public:
        elementPointer() {}
        elementPointer(null) {}

        el element() const
        {   value &Value = *This;
            return el(Index, Value);
        }

        inline value *operator -> () const
        {   if (Array == Null)
            {   throw error("array pointer is null", AT);
            }
            return Array->getNonNullValue(Index);
        }

        value &operator * () const
        {   return *(operator -> ());
        }

        bool operator == (null) const
        {   return Array == Null || Array->getValue(Index) == Null;
        }

        bool operator != (null) const
        {   return Array != Null && Array->getValue(Index) != Null;
        }

        value &otherwise(defaultTo<value> Default) const
        {   value *Value;
            if (Array != Null && (Value = Array->getValue(Index)) != Null)
            {   return *Value;
            }
            return *Default;
        }

        template <class q = value> requires (type<value>::IsConstant)
        inline q &orDefault() const
        {   return otherwise(defaultTo(type<value>::DefaultInstance));
        }

        // Removes the value from the map at this pointer's key and returns it.
        // This is technically a `const` operation on this pointer, since we don't
        // need to update the array index (compare to map::elementPointer::pop).
        template <class a = arrayPointer> requires (type<typename type<a>::pointingAt>::IsMutable)
        value pop() const
        {   if (This == Null)
            {   throw error("cannot pop from null array element pointer", AT);
            }
            return Array->pop(Index);
        }

        // increments the pointer to the next element in the array.
        // returns true if there is a next element in the array, based on the current size.
        // returns false otherwise, and sets this pointer to null.
        bool next()
        {   if (Array == Null)
            {   return False;
            }
            if (++Index >= Array->count())
            {   Array = Null;
                return False;
            }
            return True;
        }

        // Can go beyond the end of the array.
        elementPointer &operator ++() // prefix, no copy
        {   if (Array == Null)
            {   return This;
            }
            ++Index;
            return This;
        }

        elementPointer operator++ (int) // postfix, make copy
        {   elementPointer Copy = This;
            ++This;
            return Copy;
        }

        // Will make this pointer become Null if we decrement past the beginning of the array.
        elementPointer &operator --() // prefix, no copy
        {   if (Array == Null)
            {   return This;
            }
            if (--Index < 0)
            {   Array = Null;
            }
            return This;
        }

        elementPointer operator-- (int) // postfix, make copy
        {   elementPointer Copy = This;
            --This;
            return Copy;
        }
    };

    template <class arrayPointer, class v, class e>
    std::ostream &operator << (std::ostream &Out, elementPointer<arrayPointer, v, e> Pointer)
    {   Out <<
        (       type<typename type<arrayPointer>::pointingAt>::IsConstant
            ?   "array::constElementPointer("
            :   "array::elementPointer("
        );
        if (Pointer == Null)
        {   return Out << "Null)";
        }
        return Out << Pointer.element() << ")";
    }

    template <class arrayType, class iteratorValue> // e.g. (array<t> *, t &)
    class arrayIteratorKernel : public iteratorKernel<iteratorValue>
    {   index Index = -1;
        // TODO: make this a pointer<> so that we can take it in (and destroy it if need be)
        // when the array does not outlive the iterator.
        arrayType Array;

        arrayIteratorKernel(arrayType _Array)
        :   iteratorKernel<iteratorValue>
            ({  .next = [](iteratorKernel<iteratorValue> *BaseKernelSelf)
                {   CAST_DEFINE
                    (   SAFE((arrayIteratorKernel<arrayType, iteratorValue>)) *,
                        Self,
                        BaseKernelSelf
                    );
                    auto *A = Self->Array->getValue(Self->Index + 1);
                    if (A != Null)
                    {   ++Self->Index;
                        return optional<iteratorValue>(*A);
                    }
                    return optional<iteratorValue>();
                },
            }),
            Array(_Array)
        {}

    public:
        KERNEL_TO_ITERATOR
        (   arrayIteratorKernel<arrayType AND iteratorValue>,
            iteratorValue,
            (arrayType _Array),
            (_Array)
        );
    };
}

template <class t>
class array
{   std::vector<t> Internal;
    index FixedCount = -1;

    // TODO: maybe make `arrayFixedCount` its own class with a pointer to an array.
    //       it can be a view of another array or it can own the array.
    //       it'd be nice to be able to say `Array.fixedCountView()` or similar.
    // Don't want this public because it's not very clear that
    // this is a fixed size array and not just an array that
    // starts at a certain size.  use the static `array::fixedCount` function.
    // Use a second argument to ensure that array<int>({120}) gets interpreted as a vector
    // and use different types to ensure that array<int>({120, 130}) is also unambiguous.
    array(index Count, const char *)
    :   Internal(), FixedCount(Count)
    {   ASSERT(FixedCount >= 0);
        count(Count);
    }

public:
    using element = arrayElement<t &>;
    using constElement = arrayElement<const t &>;
    typedef arrayDetail::elementPointer<array *, t, element> elementPointer;
    typedef arrayDetail::elementPointer<const array *, const t, constElement> constElementPointer;

    array() : Internal() {}

    typedef t value;

    inline arrayView<t> view() &
    {   return Internal.size()
            ?   arrayView<t>(&Internal[0], &Internal[0] + Internal.size())
            :   arrayView<t>(Null, Null);
    }

    inline arrayView<const t> view() const &
    {   return Internal.size()
            ?   arrayView<const t>(&Internal[0], &Internal[0] + Internal.size())
            :   arrayView<const t>(Null, Null);
    }

    // create a fixed-size array with a given size.
    // any attempts to change the size will throw an error,
    // including getting/setting elements above the max size.
    // the fixed-size array "type" is sticky to the variable  and not passed around;
    // so `auto A = array<int>::fixedCount(10)` will always have `A` a fixed-size array
    // with 10 elements.  trying to set `A = array<int>({...})` will fail unless
    // the new array size is also 10.  similarly, a non-fixed-size array `B`
    // can be set to a fixed-size array after initialization, without changing
    // the fact that `B` is variable/non-fixed size.  e.g., `auto B = array<int>({...})`
    // and `B = array<int>::fixedCount(100)` will not then force `B` to be fixed size.
    // to convert a variable-size array to fixed size, use `fixCount()`.
    static array fixedCount(index Count)
    {   return array(Count, Null);
    }

    // makes the current array a fixed-size array.
    // idempotent if the array is already fixed size.
    void fixCount() &
    {   FixedCount = count();
    }

    bool fixedCount() const
    {   return FixedCount >= 0;
    }

    array(std::vector<t> _Vector) : Internal(std::move(_Vector)) {}

    COPYABLE_TEMPLATE
    (   array, Array,
        // FixedCount is sticky to the variable; ignore incoming FixedCount info,
        // but ensure that if the current variable is FixedCount, that we make
        // sure the incoming array is the correct size:
        if (fixedCount() && Array.count() != count())
        {   LOG_ERR("copying in a different size array, it may get truncated");
        },
        Internal = Array.Internal;
        if (FixedCount >= 0)
        {   Internal.resize(FixedCount);
        }
    )

    MOVABLE_TEMPLATE
    (   array, Array,
        // FixedCount is sticky to the variable; ignore incoming FixedCount info,
        // but ensure that if the current variable is FixedCount, that we make
        // sure the incoming array is the correct size:
        if (fixedCount() && Array.count() != count())
        {   LOG_ERR("moving in a different size array, it may get truncated");
        },
        Internal = std::move(Array.Internal);
        if (FixedCount >= 0)
        {   Internal.resize(FixedCount);
        }
        // Reset the other array's FixedCount, to be safe.
        // std::vector resets to the empty vector when moved,
        // so that would probably not be the correct FixedCount size.
        Array.FixedCount = -1;
    )

    template<class u>
    friend void std::swap(array<u>& A, array<u>& B);

    // allow implicit conversion from an iterator to an array, that's nice.
    ITERATOR_CONSTRUCTOR_TEMPLATES
    (   array, { /* TODO: reserve space if iterator has remainingCount */ },
        t, T,
        Internal.push_back(T)
    )

    ITERATOR_ASSIGNMENT_TEMPLATES
    (   array, this->count(0); { /* TODO: reserve space if iterator has remainingCount */ },
        t, T,
        Internal.push_back(T)
    )

    ITERATOR_PLUS_EQUAL_TEMPLATES
    (   array, { /* TODO: reserve EXTRA space if iterator has remainingCount */ },
        t, T,
        Internal.push_back(T)
    )

    OPERATOR_X_FROM_X_EQUALS_TEMPLATE(array, +)

    NUMBER_OPERATOR_X_EQUALS_TEMPLATE_TEMPLATE
    (   array<t>, +, template <class u>, const array<u> &,
        reserve(count() + Other.count());
        // in case we are grabbing from *this itself, e.g., X += X, grab size at start:
        const index OtherCount = Other.count();
        for (index I = 0; I < OtherCount; ++I)
        {   append(Other[I]);
        }
    )

    // TODO: allow construction from an [index, t] iterator

    // Returns a pointer to an array element that is "safe" even if this array is resized.
    // However, if this array is moved, then we have a problem.
    // Note! this will normalize negative index values before passing to the pointer.
    elementPointer get(index Index)
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index);
        return elementPointer(this, Index);
    }

    // const version of the above.
    constElementPointer get(index Index) const
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index);
        return constElementPointer(this, Index);
    }

    // TODO: this should return an UNCOPYABLE, UNMOVABLE class which can be directly set.
    // we throw if the user accesses above the size of the array.
    // when the array is non-const, we can increase the size of the array and return that value,
    // but when the array is constant we can't do that.
    // TODO: return an constArrayElement {index Index, const array *Array = this}
    // WARNING! It is not memory safe to hold onto a reference here and
    // then expand/shrink the array.
    const t &operator[] (index Index) const
    {   return *getNonNullValue(Index);
    }

    // TODO: return an arrayElement {index Index, array *Array = this}
    // WARNING! It is not memory safe to hold onto a reference here and
    // then expand/shrink the array.
    t &operator[] (index Index)
    {   return *getNonNullValue(Index);
    }

    t swap(index Index, t T)
    {   t Result = std::move(operator[](Index));
        operator[](Index) = std::move(T);
        return Result;
    }

    // Swaps indices based on their position before any changes in the array are made.
    // This is important in case one index resizes the array to be larger and the other
    // index is negative (i.e., indexing from the end of the array);
    // using the positive index position pre-swap makes the argument order not matter.
    void swapIndices(index Index1, index Index2)
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index1);
        ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index2);
        t &Value1 = operator[](Index1);
        t &Value2 = operator[](Index2);
        // Avoid move-assigning to self during the swap:
        if (&Value1 == &Value2)
            return;
        std::swap(Value1, Value2);
    }

    inline t &append(t &&NewEnd) &
    {   if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        index NewIndex = count();
        Internal.push_back(std::move(NewEnd));
        return Internal[NewIndex];
    }

    template <class u = t> requires (type<u>::IsCopyConstructible)
    inline t &append(const u &NewEnd) &
    {   if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        index NewIndex = count();
        Internal.push_back(NewEnd);
        return Internal[NewIndex];
    }

    // TODO: rename to `appendConstruct`
    template<class... arguments>
    inline t &appendInPlace(arguments&&... Arguments) &
    {   if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        index NewIndex = count();
        Internal.emplace_back(std::forward<arguments>(Arguments)...);
        return Internal[NewIndex];
    }

    inline t pop()
    {   if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        const index Count = count();
        if (Count <= 0)
            throw error(ArrayPoppingErrorMsg, AT);
        t Result = std::move(Internal[Count - 1]);
        Internal.pop_back();
        return Result;
    }

    inline t pop(index Index)
    {   if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index)
        else if (Index >= count())
        {   throw error(ArrayPoppingErrorMsg, AT);
        }
        t Result = std::move(Internal[Index]);
        auto Begin = Internal.begin() + Index;
        Internal.erase(Begin, Begin + 1);
        return Result;
    }

    void insert(index Index, t Value, index Count = 1)
    {   if (Count == 0)
            return;
        if (Count < 0)
            throw error(ArrayInvalidArgumentErrorMsg, AT);
        if (fixedCount())
            throw error(ArrayFixedCountErrorMsg, AT);
        if (Index < 0)
        {   Index += count();
            if (Index < 0)
                throw error(ArrayTooNegativeIndexErrorMsg, AT);
        }
        else if (Index >= count())
        {   count(Index);
        }
        Internal.insert(Internal.begin() + Index, Count, Value);
    }

    // Removes all elements in the Array that are equal to the passed in value.
    // Returns the number of elements removed.
    index remove(const t &T)
    {   if (fixedCount())
        {   throw error(ArrayFixedCountErrorMsg, AT);
        }
        auto Iterator = Internal.begin();
        index Removed = 0;
        while (Iterator != Internal.end())
        {   if (*Iterator == T)
            {   Iterator = Internal.erase(Iterator, Iterator + 1);
                ++Removed;
            }
            else
            {   ++Iterator;
            }
        }
        return Removed;
    }

    inline void erase(index Index)
    {   erase(Index, 1);
    }

    void erase(index StartingFrom, index Count)
    {   // Erases elements from index `StartingFrom` to index `StartingFrom + Count - 1`.
        // TODO: this silently will ignore a "failure" to erase Count if we hit the end of the array first:
        // is that ok?
        if (Count == 0)
            return;
        if (Count < 0)
            throw error(ArrayInvalidArgumentErrorMsg, AT);
        if (fixedCount())
        {   throw error(ArrayFixedCountErrorMsg, AT);
        }
        if (StartingFrom < 0) {
            StartingFrom += count();
            if (StartingFrom < 0)
                throw error(ArrayTooNegativeIndexErrorMsg, AT);
        }
        auto Begin = Internal.begin() + StartingFrom;
        Internal.erase(Begin, Begin + Count);
    }

    // Returns true iff we found an element that compares equal to the passed-in element;
    // If found, the passed-in pointer will be set to the index of the element in the array.
    bool findFirst(const t &T, determining<index> Index) const
    {   index Count = count();
        for (index I = 0; I < Count; ++I)
        {   if (Internal[I] == T)
            {   Index = I;
                return True;
            }
        }
        return False;
    }

    // TODO: `reverse() const` that returns a copy of the array (but reversed)
    array<t> &reverse()
    {   std::reverse(Internal.begin(), Internal.end());
        return This;
    }

    // TODO: `sort() const` that returns a copy of the array (but sorted)
    array<t> &sort()
    {   std::sort(Internal.begin(), Internal.end());
        return *this;
    }

    // TODO: sort with a comparison function

    inline bool empty() const
    {   return count() == 0;
    }

    inline index count() const
    {   ASSERT(Internal.size() <= INT64_MAX);
        return Internal.size();
    }

    inline void count(index ResizeTo)
    {   ASSERT(ResizeTo >= 0);
        if (fixedCount() && ResizeTo != FixedCount)
        {   throw error(ArrayFixedCountErrorMsg, AT);
        }
        Internal.resize(ResizeTo);
    }

    // Removes all elements from the array but does not reclaim any memory
    // for a normal array.  For a fixed-size array, sets all values to the default value.
    inline void clear()
    {   if (fixedCount())
        {   ASSERT(FixedCount == count());
            for (index I = 0; I < FixedCount; ++I)
            {   Internal[I] = t();
            }
        }
        else
        {   Internal.clear();
        }
    }

    // Clears the array and frees memory.  Not safe for a fixed-size array.
    // It is safe to continue using the array after this call, and it will start empty.
    inline void deallocate()
    {   if (fixedCount())
        {   throw error(ArrayFixedCountErrorMsg, AT);
        }
        std::vector<t> NewInternal;
        std::swap(NewInternal, Internal);
    }

    inline void reserve(index Count)
    {   ASSERT(Count >= 0);
        if (fixedCount() && Count > FixedCount)
        {   throw error(ArrayFixedCountErrorMsg, AT);
        }
        Internal.reserve(Count);
    }

    // Note we ignore whether the array is FixedCount
    // when comparing.  This is because FixedCount arrays
    // are "sticky" to the variable -- e.g., copying a FixedCount array
    // will not necessarily give a FixedCount array.  Default is no.
    bool operator == (const array &Other) const
    {   if (count() != Other.count())
            return False;
        index Count = count();
        for (index I = 0; I < Count; ++I)
        {   if (operator[](I) != Other[I])
            {   return False;
            }
        }
        return True;
    }

    inline bool operator != (const array &Other) const
    {   return !(*this == Other);
    }

    // TODO: create rvalue iterators, e.g., for `values() &` and `values() &&`
    // the latter needs to take the array memory into itself for iterating,
    // since the array is temporary.
    iterator<t &> values() &
    {   return arrayDetail::arrayIteratorKernel<array<t> *, t &>::toIterator(this);
    }
    
    iterator<const t &> values() const &
    {   return arrayDetail::arrayIteratorKernel<const array<t> *, const t &>::toIterator(this);
    }

    elementPointer first() &
    {   return empty() ? elementPointer() : elementPointer(this, 0);
    }

    constElementPointer first() const &
    {   return empty() ? constElementPointer() : constElementPointer(this, 0);
    }

    // TODO: last() and last() const

    CONTAINER_ITERATOR(array)
    MUTABLE_CONTAINER_ITERATOR(array)

    // TODO: maybe add keys() or indices() iterator
private:
    t *getValue(index Index)
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index)
        else if (Index >= count())
        {   return Null;
        } 
        return &Internal[Index];
    }

    // const version of the above.
    const t *getValue(index Index) const
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index)
        else if (Index >= count())
        {   return Null;
        } 
        return &Internal[Index];
    }

    inline t *getNonNullValue(index Index)
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index)
        else if (Index >= count())
        {   // This will throw if the array is fixed size:
            TRY("", count(Index + 1));
        }
        ASSERT(Index >= 0 && Index < count());
        return &Internal[Index];
    }

    // const version of the above.
    inline const t *getNonNullValue(index Index) const
    {   ARRAY_ADJUST_INDEX_IF_NEGATIVE(Index)
        else if (Index >= count())
        {   throw error(ArrayFixedCountErrorMsg, AT);
        } 
        return &Internal[Index];
    }

    template <class a1, class t1, class e1>
    friend class arrayDetail::elementPointer;
    template <class a1, class t1>
    friend class arrayDetail::arrayIteratorKernel;
};

template <class t>
std::ostream &operator << (std::ostream &Out, const array<t> &Array)
{   Out << "array({ ";
    const index Count = Array.count();
    for (index I = 0; I < Count; ++I)
        Out << Array[I] << ", ";
    return Out << "})";
}

#ifndef NDEBUG
template <class t, class u>
bool checkEqual(const array<t> &T, const array<u> &U)
{   index Count = T.count();
    if (U.count() != Count) return false;
    for (index I = 0; I < Count; ++I)
    {   if (!checkEqual(T[I], U[I]))
            return false;
    }
    return true;
}
#endif

TMVB

namespace std
{   template<class t>
    void swap(bvmt::array<t>& A, bvmt::array<t>& B)
    {   if (A.fixedCount() || B.fixedCount())
        {   // But if either of them is fixed size, the sizes
            // need to be compatible:
            if (A.count() != B.count())
                throw bvmt::error(bvmt::ArrayFixedCountErrorMsg, AT);
        }
        std::swap(A.Internal, B.Internal);
    }

    template<class t>
    struct hash<bvmt::array<t>>
    {   std::size_t operator() (const bvmt::array<t>& Array) const
        {   size_t Result = 17;
            for (const t &T : Array.values())
            {   Result = Result * 31 + hash<t>()(T);
            }
            return Result;
        }
    };
}
