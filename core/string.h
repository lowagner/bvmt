#pragma once

#include "arg.h"
#include "error.h"
#include "iterator.h"
#include "types.h"

#include <sstream> 
#include <string>

#define ASSERT_STRING(X, y) ASSERT_THIS(bvmt::string(X), y)

BVMT

namespace detail
{   class stringIteratorKernel;
    class stringSplitIteratorKernel;
}

class string;
class stringView;
class stringAsciiCompare;

#define STRING_LIKE_H() \
    /* Returns the utf8 rune at the start of this stringView, or 0 if the stringView is empty.
       Note that a 0 here does not imply that this stringView is empty, however. */ \
    rune first() const; \
    \
    /* Returns the utf8 rune at the end of this stringView, or 0.
       Note that a 0 here does not imply that this stringView is empty, however. */ \
    rune last() const;

#define STRING_LIKE_CC(stringLike) \
    rune stringLike::first() const \
    {   stringView Copy = This; \
        return Copy.shift(); \
    } \
    \
    rune stringLike::last() const \
    {   stringView Copy = This; \
        return Copy.pop(); \
    } \

#define STRING_LIKE_TEMPLATES() \
    template <class t> \
    inline string operator + (t T) const \
    {   string Result = This; \
        Result += T; \
        return Result; \
    } \
    \
    template <class t> \
    inline string operator * (t T) const \
    {   string Result; \
        /* TODO: make a special function that you can overload for your types: */ \
        index I = T; \
        if (I > 0) \
        {   Result.reserve(I * countBytes()); \
            while (I > 0) \
            {   Result += This; \
                --I; \
            } \
        } \
        return Result;  \
    } \
    \
    private: \
    template <class t> \
    inline string enclose(char Open, t T, char Close) const \
    {   string Result; \
        Result.reserve(countBytes() + 2 + T.countBytes()); \
        Result += This; \
        Result += Open; \
        Result += T; \
        Result += Close; \
        return Result; \
    } \
    \
    public: \
    template <class t> \
    inline string operator [] (t T) const \
    {   return enclose('[', T, ']'); \
    } \
    \
    template <class t> \
    inline string operator () (t T) const \
    {   return enclose('(', T, ')'); \
    } \
    template <class t> \
    inline string templateWith(t T) const \
    {   return enclose('<', T, '>'); \
    }

// TODO: maybe switch string to using a localArray to see if it "speeds things up".
// probably best to use `stringLocal` and see if that works in every use-case.
// we probably want something like localArray<u8, u16 (alloc)> which switches over
// to a real string if it goes past max alloc.  maybe we can build this into localArray;
// e.g., if the size is too big, then use a pointer to the data array, with two index (i64)
// values allocated before it for size and alloc.
class string
{   std::string Internal;
public:
    // sets the locale to the passed in value, e.g., "en_US.utf8".
    static void locale(const char *Chars);

    string(const char *Chars = "");
    string(std::string String);
    explicit string(rune Rune);
    string(stringView StringView);

    template <class t>
    static string of(t Value)
    {   if constexpr (type<t>::$ equals<string>())
        {   return Value;
        }
        else
        {   std::ostringstream Stream;
            Stream.imbue(std::locale("C"));
            Stream << Value;
            return Stream.str();
        }
    }

    stringView view() const &;

    bool empty() const;

    const char *chars() const &;

    void append(rune Rune) &;
    void append(const string &String) &;
    // TODO: add append(iterator<rune> &&)

    template <class t>
    inline string &operator += (t T)
    {   append(T);
        return *this;
    }

    template <class t>
    inline string &operator *= (t T)
    {   string Result = This * T;
        std::swap(This, Result);
        return This;
    }

    // TODO: overload for string + string which reserves space ahead of time

    // TODO: `operator -` but will throw an error if the value isn't on the end of the string.
    // will remove it if it's present.

    rune pop();

    // Returns the size (in utf8 characters, i.e., runes) of the string.
    // Note, this is not O(1).
    // TODO: maybe make O(1) by determining rune size on init, keeping invariant up to date
    // with append/pop/etc.
    index count() const;

    // Reserves this many bytes for the string data.
    void reserve(index Bytes);

    bool contains(const char *Substring) const;
    // TODO: bool contains(const stringView &Substring) const;
    bool contains(const string &Substring) const;

    bool operator == (const char *Other) const;
    bool operator == (const std::string &Other) const;
    bool operator == (const string &Other) const;
    bool operator == (const stringView &Other) const;

    bool operator < (const string &Other) const;

    template <class t>
    inline bool operator != (t Other) const
    {   return !(This == Other);
    }

    iterator<rune> runes() const;

    STRING_LIKE_H()
    STRING_LIKE_TEMPLATES()

// TODO: reset this to `protected:` after we switch to u8 arrays in server.h
    index countBytes() const;

    template <class t>
    bool integer(determining<t> Out) const &
    {   return view().integer(Out); // TODO: avoid compiler warnings
    }

    template <class t>
    bool real(determining<t> Out) const &
    {   return view().real(Out); // TODO: avoid compiler warnings
    }

private:
    friend stringAsciiCompare;
    friend stringView;
    friend class detail::stringIteratorKernel;
    friend class ::std::hash<bvmt::string>;
    friend std::ostream &operator << (std::ostream &Out, const string &String);
};

std::ostream &operator << (std::ostream &Out, const string &String);

struct stringAsciiCompare
{   bool operator() (const string& A, const string& B) const
    {   return A.Internal < B.Internal;
    }
};

class stringView
{   // TODO: switch to using std::basic_string_view for ease of algo, like contains() -> find()
    // TODO: or maybe switch to using a `const string *Internal` snapshot.
    // TODO: or maybe, like arrayView, this should be a generic `char *`.
    const std::string *Internal;
    index StartByte;
    index EndByte;

    stringView(const string &String, index _StartByte, index _EndByte);
public:
    stringView();
    stringView(const string &String);

    // Makes a copy of this stringView:
    stringView view() const;

    bool empty() const;

    /* TODO
    bool startsWith(stringView StringView) const;
    */

    // returns the next utf8 rune at the start of the stringView (i.e., `first()`),
    // while incrementing the start byte of this stringView to after the returned rune.
    // returns 0 if the stringView is empty, but a zero does not guarantee the stringView is empty().
    rune shift();

    // returns the utf8 rune at the end of the stringView,
    // while decrementing the end byte of this stringView to before the returned rune.
    // returns 0 if the stringView is empty, but a zero does not guarantee the stringView is empty().
    rune pop();

    // Removes whitespace on the front or back of the stringView.
    // Currently only does spaces and newlines.
    stringView &strip() &;
    stringView strip() &&;

    // Removes whitespace on the front of the stringView.
    // Currently only does spaces and newlines.
    stringView &stripFront() &;
    stringView stripFront() &&;

    // Removes whitespace on the back of the stringView.
    // Currently only does spaces and newlines.
    stringView &stripBack() &;
    stringView stripBack() &&;

    // Returns the size (in number of utf8 characters, i.e., runes) of what's in this string view.
    // Note, this is not O(1).
    index count() const;

    // TODO: bool contains(const char *Substring) const;
    // TODO: bool contains(const stringView &Substring) const;
    // TODO: bool contains(const string &Substring) const;

    bool operator == (const char *Other) const;
    bool operator == (const string &Other) const;
    bool operator == (const stringView &Other) const;

    template <class t>
    inline bool operator != (t Other) const
    {   return !(This == Other);
    }

    // TODO: overloads for `runes() &` and `runes() &&`
    iterator<rune> runes() const;

    iterator<stringView> split(rune Split) const;

    // Consumes the starting bytes if they are integer-like (0-9), updating the passed-in number.
    // Returns true if this string started with an integer; if the whole stringView should be
    // a integer, use `integer` instead.  Does not update the passed-in number if this
    // stringView was not an integer.  Also accepts a + or - prefix, the latter of course makes
    // the passed-in number negative.  Also accepts binary strings that start with `0b`, e.g.,
    // 0b110 is 6.
    template <class t>
    bool shiftInteger(determining<t> Out)
    {   // TODO: check std::numeric_limits<t>::min()/max() to ensure we're in bounds
        // will become non-null if we write to Out:
        t *T = Null;
        if (empty())
        {   return False;
        }
        const index InitialStartByte = StartByte;
        bool Negative = False;
        i8 Byte = Internal->operator[](StartByte);
        if (Byte == '-')
        {   Negative = True;
            ++StartByte;
        }
        else if (Byte == '+')
        {   ++StartByte;
        }
        index Position = 0;
        int Base = 10;
        i8 MaxDigit = '9';
        while (!empty())
        {   Byte = Internal->operator[](StartByte);
            if (Byte < '0' || Byte > MaxDigit)
            {   if (Position == 1)
                {   if (Byte == 'b')
                    {   ++StartByte;
                        Base = 2;
                        MaxDigit = '1';
                        continue;
                    }
                }
                break;
            }
            ++Position;
            if (T == Null)
            {   T = &(Out = 0);
            }
            ++StartByte;
            *T = *T * Base + (Byte - '0');
        }
        // In case we grabbed a leading + or - but found no subsequent digits:
        if (T == Null)
        {   StartByte = InitialStartByte;
        }
        else if (Negative)
        {   *T *= -1;
        }
        return T != Null;
    }

    // Consumes the full stringView if the stringView is just an integer, returning
    // True and updating the passed-in pointer to the number.  If False, the stringView is reset.
    // Like `shiftInteger` but also returns False if the stringView continues after the number
    // with non-numeric runes.  Moves in and out the passed-in number value to reset if necessary.
    template <class t>
    bool integer(determining<t> Out) &
    {   auto Resettable = Out.resettable();
        const index InitialStartByte = StartByte;
        bool ValidInteger = shiftInteger(Out);
        if (ValidInteger && !empty())
        {   // string started with a number, but had other stuff afterwards; no longer just an integer:
            ValidInteger = False;
        }
        if (ValidInteger)
        {   Resettable.doNotResetOnDescope();
        }
        else
        {   StartByte = InitialStartByte;
        }
        return ValidInteger; 
    }

    // Returns true if this entire stringView is an integer, without changing the stringView itself.
    // Sets the passed-in pointer to the value of the integer, if the stringView is an integer.
    template <class t>
    bool integer(determining<t> Out) const &
    {   stringView Copy = This;
        return Copy.integer(Out);
    }

    // Consumes the starting bytes if they are real-like ([0-9]+{.[0-9]*}?) with an optional
    // prefix + or -, setting the passed-in pointer to the value if so.  If the whole
    // stringView should be a real number, use `real` instead.  Will greedily accept "." and
    // will ignore any second period.  E.g., "9.5.4" will become the float 9.5 and the remaining
    // stringView will be ".4".  Note that the number (after any optional + or -) must
    // start with a digit, so if there is no 1's digit, use "0." to start the decimal expansion.
    // TODO: allow XeY where X and Y are decimals, and X has an optional period
    template <class t>
    bool shiftReal(determining<t> Out)
    {   if (empty())
        {   return False;
        }
        const index InitialStartByte = StartByte;
        t *T = Null;
        bool FoundPeriod = False;
        t Decimals = t(1);
        bool Negative = False;
        i8 Byte = Internal->operator[](StartByte);
        if (Byte == '-')
        {   Negative = True;
            ++StartByte;
        }
        else if (Byte == '+')
        {   ++StartByte;
        }
        while (!empty())
        {   Byte = Internal->operator[](StartByte);
            if (Byte < '0' || Byte > '9')
            {   if (Byte == '.')
                {   if (T == Null || FoundPeriod)
                    {   // either (1) we haven't found a number digit yet,
                        // or (2) we did and found a second period; either way, quit:
                        break;
                    }
                    FoundPeriod = True;
                    ++StartByte;
                    continue;
                }
                else
                {   break;
                }
            }
            ++StartByte;
            if (T == Null)
            {   T = &(Out = 0);
            }
            if (FoundPeriod)
            {   Decimals /= 10;
                DEBUG_ONLY
                (   if (Decimals == 0)
                    {   LOG_ERR("numerical precision has been lost, too many digits past the decimal");
                        // don't break, need to consume all number digits here.
                    }
                );
                *T += Decimals * (Byte - '0');
            }
            else
            {   *T = *T * 10 + (Byte - '0');
            }
        }
        // In case we grabbed a leading + or - but found no subsequent digits:
        if (T == Null)
        {   StartByte = InitialStartByte;
        }
        else if (Negative)
        {   *T *= -1;
        }
        return T != Null;
    }

    // Consumes the full stringView if the stringView is just a real, returning
    // True and updating the passed-in pointer to the number.  If False, the stringView is reset.
    // Like `shiftReal` but also returns False if the stringView continues after the number
    // with non-numeric runes.  Moves in and out the passed-in number value to reset if necessary.
    template <class t>
    bool real(determining<t> Out) &
    {   auto Resettable = Out.resettable();
        const index InitialStartByte = StartByte;
        bool ValidReal = shiftReal(Out);
        if (ValidReal && !empty())
        {   // string started with a number, but had other stuff afterwards; no longer just an integer:
            ValidReal = False;
        }
        if (ValidReal)
        {   Resettable.doNotResetOnDescope();
        }
        else
        {   StartByte = InitialStartByte;
        }
        return ValidReal; 
    }

    // Returns true if this entire stringView is a real, without changing the stringView itself.
    // Sets the passed-in pointer to the value of the real, if the stringView is a real.
    template <class t>
    bool real(determining<t> Out) const &
    {   stringView Copy = This;
        return Copy.real(Out);
    }

    STRING_LIKE_H()
    STRING_LIKE_TEMPLATES()

// TODO: reset this to `protected:` after we switch to u8 arrays in server.h
    index countBytes() const;

private:
    rune shiftNotEmpty();
    u8 shiftByteNotEmpty();
    
    u8 popByteNotEmpty();

    friend string;
    friend class detail::stringIteratorKernel; 
    friend class detail::stringSplitIteratorKernel;
    friend std::ostream &operator << (std::ostream &Out, const stringView &StringView);
};

std::ostream &operator << (std::ostream &Out, const stringView &StringView);

TMVB

namespace std
{   template <>
    struct hash<bvmt::string>
    {   std::size_t operator() (const bvmt::string& String) const;
    };
}

#endif
