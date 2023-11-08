#include "string.h"

#ifndef NDEBUG
#include "array.h"
#endif

#include <locale>

BVMT

STRING_LIKE_CC(string)

void string::locale(const char *Chars)
{   std::locale::global(std::locale(Chars));
}

string::string(const char *Chars) : Internal(Chars) {}

string::string(std::string String) : Internal(std::move(String)) {}

string::string(rune Rune)
{   append(Rune);
}

string::string(stringView StringView)
:   Internal
    (   StringView.Internal->begin() + StringView.StartByte,
        StringView.Internal->begin() + StringView.EndByte
    )
{}

stringView string::view() const &
{   return stringView(This);
}

bool string::empty() const
{   return Internal.size() == 0;
}

const char *string::chars() const &
{   return Internal.c_str();
}

void string::append(rune Rune) &
{   if (Rune < 0)
    {   LOG_ERR("Rune < 0");
    }
    else if (Rune < 128)
    {   Internal.push_back((char)Rune);
    }
    else if (Rune < 2048)
    {   Internal.push_back(0b11000000 | (Rune >> 6));
        Internal.push_back(0b10000000 | (0b00111111 & Rune));
    }
    else if (Rune < 65536)
    {   Internal.push_back(0b11100000 | (Rune >> 12));
        Internal.push_back(0b10000000 | (0b00111111 & (Rune >> 6)));
        Internal.push_back(0b10000000 | (0b00111111 & Rune));
    }
    else if (Rune < 1114112)
    {   Internal.push_back(0b11110000 | (Rune >> 18));
        Internal.push_back(0b10000000 | (0b00111111 & (Rune >> 12)));
        Internal.push_back(0b10000000 | (0b00111111 & (Rune >> 6)));
        Internal.push_back(0b10000000 | (0b00111111 & Rune));
    }
    else
    {   LOG_ERR("Rune probably not unicode.");
    }
}

void string::append(const string &Other) &
{   Internal += Other.Internal;
}

rune string::pop()
{   stringView SelfView = view();
    rune Result = SelfView.pop();
    ASSERT(SelfView.EndByte >= 0 && SelfView.EndByte <= countBytes());
    // Make sure to resize the current array, the stringView doesn't do that:
    Internal.resize(SelfView.EndByte);
    return Result;
}

index string::count() const
{   // TODO: possibly keep track of this separately, so this operation is O(1) instead of O(N)
    return view().count();
}

index string::countBytes() const
{   return Internal.size();
}

void string::reserve(index Bytes)
{   Internal.reserve(Bytes);
}

bool string::contains(const char *Substring) const
{   return Internal.find(Substring, 0) != std::string::npos;
}

bool string::contains(const string &Substring) const
{   return Internal.find(Substring.Internal, 0) != std::string::npos;
}

bool string::operator == (const char *Other) const
{   return Internal == Other;
}

bool string::operator == (const std::string &Other) const
{   return Internal == Other;
}

bool string::operator == (const string &Other) const
{   return Internal == Other.Internal;
}

bool string::operator == (const stringView &Other) const
{   return view() == Other;
}

bool string::operator < (const string &Other) const
{   std::locale Locale; // get current default locale
    auto& Facet = std::use_facet<std::collate<char>>(Locale);
 
    return Facet.compare
    (   &Internal[0], &Internal[0] + Internal.size(),
        &(Other.Internal[0]), &(Other.Internal[0]) + Other.Internal.size()
    )   < 0;
}

std::ostream &operator << (std::ostream &Out, const string &String)
{   return Out << String.Internal;
}

STRING_LIKE_CC(stringView)

stringView::stringView()
:   stringView(type<string>::DefaultInstance, 0, 0)
{}

stringView::stringView(const string &String, index _StartByte, index _EndByte)
:   Internal(&String.Internal),
    StartByte(_StartByte),
    EndByte(_EndByte)
{   ASSERT(EndByte >= StartByte);
    ASSERT(StartByte >= 0);
}

stringView::stringView(const string &String)
:   stringView(String, 0, String.Internal.size())
{}

stringView stringView::view() const
{   return This;
}

bool stringView::empty() const
{   ASSERT(StartByte >= 0);
    return
            StartByte >= EndByte
            // TODO: we should be able to remove this condition by ensuring StartByte >= EndByte
            // in the situation where this might occur.
        ||  StartByte >= (index)Internal->size()
    ;
}

rune stringView::shift()
{   return empty() ? 0 : shiftNotEmpty();
}

rune stringView::pop()
{   if (empty())
    {   return 0;
    }
    rune Result = popByteNotEmpty();

    if (!(Result & 128)) // (StandardAscii & 128) equals 0
    {   return Result;
    }
    if ((Result & 192) != 128) // (UtfContinuation & 64) should be 0
    {   return -1;
    }
    Result &= 0b00111111;
    // TODO: UnmaskBits could probably be optimized away by shifting (uint8_t)((uint8_t)(-1) << X)
    u8 UnmaskBits = 0b11000000; // starts at 128 + 64, but add in 32, 16 as necessary as we go back.
    for (int I = 1; I <= 3; ++I)
    {   if (empty())
        {   return -1;
        }
        u8 NextByte = popByteNotEmpty();
        if ((NextByte & 192) != 128)
        {   ASSERT((NextByte & UnmaskBits) == UnmaskBits);
            Result += (NextByte & ~UnmaskBits) << (6 * I);
            return Result;
        }
        UnmaskBits |= 1 << (6 - I);
        Result += (NextByte & 0b00111111) << (6 * I);
    }
    return -1;
}

stringView &stringView::strip() &
{   stripFront();
    stripBack();
    return This;
}

stringView stringView::strip() &&
{   stringView NewView = std::move(This);
    NewView.stripFront();
    NewView.stripBack();
    return NewView;
}

stringView &stringView::stripFront() &
{   while (True)
    {   index StartByteToRestoreIfNecessary = StartByte;
        // ok if empty(), this will return 0:
        rune StartRune = shift();
        switch (StartRune)
        {   case ' ':
            case '\n':
                break;
            default:
                StartByte = StartByteToRestoreIfNecessary;
                return This;
        }
    }
    // Shouldn't ever get here, but compiler warns about it.
    return This;
}

// TODO: why do we need this?
stringView stringView::stripFront() &&
{   stringView NewView = std::move(This);
    NewView.stripFront();
    return NewView;
}

stringView &stringView::stripBack() &
{   while (True)
    {   index EndByteToRestoreIfNecessary = EndByte;
        // ok if empty(), this will return 0:
        rune EndRune = pop();
        switch (EndRune)
        {   case ' ':
            case '\n':
                break;
            default:
                EndByte = EndByteToRestoreIfNecessary;
                return This;
        }
    }
    // Shouldn't ever get here, but compiler warns about it.
    return This;
}

stringView stringView::stripBack() &&
{   stringView NewView = std::move(This);
    NewView.stripBack();
    return NewView;
}


index stringView::count() const
{   stringView Copy = *this;
    index Size = 0;
    while (!Copy.empty())
    {   ++Size;
        Copy.shift();
    }
    return Size;
}

index stringView::countBytes() const
{   return std::max(0L, std::min((index)(Internal->size()), EndByte) - StartByte);
}

bool stringView::operator == (const char *Other) const
{   stringView Copy = *this;
    while (True)
    {   if (Copy.empty())
        {   if (*Other == 0)
            {   return True;
            }
            return False;
        }
        else if (*Other == 0)
        {   return False;
        }
        if (Copy.shiftByteNotEmpty() != (u8)*(Other++))
        {   return False;
        }
    }
    // Shouldn't ever get here, but compiler warns about it.
    return False;
}

bool stringView::operator == (const string &Other) const
{   return *this == Other.view();
}

bool stringView::operator == (const stringView &Other) const
{   stringView Copy = *this;
    stringView OtherCopy = Other;
    while (True)
    {   if (Copy.empty())
        {   if (OtherCopy.empty())
            {   return True;
            }
            // Copy is empty but OtherCopy is not...
            return False;
        }
        else if (OtherCopy.empty())
        {   // Copy is not empty, but OtherCopy is...
            return False;
        }
        if (Copy.shiftByteNotEmpty() != OtherCopy.shiftByteNotEmpty())
        {   return False;
        }
    }
    // Shouldn't ever get here, but compiler warns about it.
    return False;
}

std::ostream &operator << (std::ostream &Out, const stringView &StringView)
{   stringView Copy = StringView;
    while (!Copy.empty())
    {   Out << (char)Copy.shiftByteNotEmpty();
    }
    return Out;
}

rune stringView::shiftNotEmpty()
{   rune Rune = shiftByteNotEmpty();

    if (!(Rune & 128))
    {   // UTF8 sequence began with byte (0b0xxxxxxx),
        // this is ASCII, consume no more bytes.
        return Rune;
    }
    if (!(Rune & 64))
    {   LOG_ERR("sequence began with UTF8 continuation byte (0b10xxxxxx)");
        return -1;
    }
    int ExtraBytes;
    if (!(Rune & 32))
    {   // UTF8 sequence began with byte (0b110xxxxx), consume 2 bytes total, one more:
        Rune &= 0b00011111;
        ExtraBytes = 1;
    }
    else if (!(Rune & 16))
    {   // UTF8 sequence began with byte (0b1110xxxx), consume 3 bytes total, two more:
        Rune &= 0b00001111;
        ExtraBytes = 2;
    }
    else if (!(Rune & 8))
    {   // UTF8 sequence began with byte (0b11110xxx), consume 4 bytes total, three more:
        Rune &= 0b00000111;
        ExtraBytes = 3;
    }
    else
    {   LOG_ERR("sequence began with invalid UTF8 byte (0b11111xxx)");
        return -1;
    }

    while (--ExtraBytes >= 0)
    {   if (empty())
        {   LOG_ERR("string ended before expected UTF8 sequence did");
            return -1;
        }
        u8 NextByte = shiftByteNotEmpty();
        ASSERT
        (   (NextByte & 192) == 128
            /* UTF8 sequence continuation byte looks like 0b10xxxxxx */
        );
        Rune = (Rune << 6) + (NextByte & 0b00111111);
    }
    return Rune;
}

u8 stringView::shiftByteNotEmpty()
{   ASSERT(!empty());
    return (u8)Internal->operator[](StartByte++);
}

u8 stringView::popByteNotEmpty()
{   ASSERT(!empty());
    return (u8)Internal->operator[](--EndByte);
}

// TODO: switch to using CONTAINER_ITERATOR with `stringPointer`
namespace detail
{   class stringIteratorKernel : public iteratorKernel<rune>
    {   stringView StringView;
    private:
        inline rune shiftRuneNotEmpty()
        {   return StringView.shiftNotEmpty();
        }

        stringIteratorKernel(const stringView _StringView)
        :   iteratorKernel<rune>
            ({  .next = [](iteratorKernel<rune> *BaseKernelSelf)
                {   CAST_DEFINE
                    (   stringIteratorKernel *,
                        Self,
                        BaseKernelSelf
                    );
                    if (Self->StringView.empty())
                    {   return optional<rune>();
                    }
                    return optional<rune>(Self->shiftRuneNotEmpty());
                },
            }),
            StringView(_StringView)
        {}

    public:
        KERNEL_TO_ITERATOR
        (   stringIteratorKernel,
            rune,
            (stringView _StringView),
            (_StringView)
        );
    };

    class stringSplitIteratorKernel : public iteratorKernel<stringView>
    {   stringView RemainingView;
        index EndOfLastRegionByte;
        rune SplitRune;
    private:
        inline bool hasNext()
        {   return EndOfLastRegionByte < RemainingView.EndByte;
        }

        inline stringView findNextSplit()
        {   ASSERT(hasNext());
            stringView Result = RemainingView;
            while (True)
            {   if (RemainingView.empty())
                {   ASSERT(RemainingView.StartByte == Result.EndByte);
                    EndOfLastRegionByte = RemainingView.StartByte;
                    return Result;
                }
                index PossibleEndOfRegionByte = RemainingView.StartByte;
                rune NextRune = RemainingView.shift();
                if (NextRune == SplitRune)
                {   Result.EndByte = PossibleEndOfRegionByte;
                    EndOfLastRegionByte = PossibleEndOfRegionByte;
                    return Result;
                }
            }
            // Shouldn't ever get here, but compiler warns about it.
            return Result;
        }

        stringSplitIteratorKernel(stringView StringView, rune _SplitRune)
        :   iteratorKernel<stringView>
            ({  .next = [](iteratorKernel<stringView> *BaseKernelSelf)
                {   CAST_DEFINE
                    (   stringSplitIteratorKernel *,
                        Self,
                        BaseKernelSelf
                    );
                    return Self->hasNext()
                            ? optional<stringView>(Self->findNextSplit())
                            : optional<stringView>();
                },
            }),
            RemainingView(StringView),
            EndOfLastRegionByte(StringView.StartByte - 1),
            SplitRune(_SplitRune)
        {}

    public:
        KERNEL_TO_ITERATOR
        (   stringSplitIteratorKernel,
            stringView,
            (stringView StringView, rune _SplitRune),
            (StringView, _SplitRune)
        );
    };
}

iterator<rune> string::runes() const
{   return view().runes();
}

iterator<rune> stringView::runes() const
{   return detail::stringIteratorKernel::toIterator(*this);
}

iterator<stringView> stringView::split(rune Split) const
{   return detail::stringSplitIteratorKernel::toIterator(*this, Split);
}

#ifndef NDEBUG
void test__core__string()
{   string::locale("en_US.utf8"); // for utf8 support

    TEST(
        // Default string is empty:
        string String;
        EXPECT_EQUAL(String, "");
        EXPECT_EQUAL(String.chars(), "");

        std::string Empty;
        EXPECT_EQUAL(String, Empty);
    );

    TEST(
        // Other strings work as expected:
        string String("asdf");
        EXPECT_EQUAL(String, "asdf");

        string OtherString("not asdf");
        EXPECT_EQUAL(OtherString, "not asdf");

        EXPECT_NOT_EQUAL(String, OtherString);
    );

    TEST(
        // Using std::string to initialize:
        std::string CxxString = "hello!";
        string String(CxxString);
        EXPECT_EQUAL(String, "hello!");

        CxxString = "Cranberries!";
        string OtherString(CxxString);
        EXPECT_EQUAL(OtherString, "Cranberries!");
        EXPECT_NOT_EQUAL(String, OtherString);

        // And for common sense, the first string shouldn't change:
        EXPECT_EQUAL(String, "hello!");
    );

    TEST(
        // Using rune to initialize:
        EXPECT_EQUAL(string(127820), "🍌");
        EXPECT_EQUAL(string(223), "ß");
        EXPECT_EQUAL(string('Z'), "Z");
    );

    TEST
    (   // can construct from a stringView
        string String("Cerberus");
        stringView StringView = String.view();
        EXPECT_EQUAL(StringView, String);

        StringView.shift();
        
        EXPECT_NOT_EQUAL(StringView, String);

        string NewString(StringView);
        EXPECT_EQUAL(NewString, StringView);
        EXPECT_EQUAL(NewString, "erberus");

        EXPECT_NOT_EQUAL(String, NewString);

        // no back reaction on parent string (NewString has its own memory):
        NewString.pop();
        NewString.append('q');
        EXPECT_EQUAL(NewString, "erberuq");
        EXPECT_EQUAL(String, "Cerberus");
    );

    TEST
    (   // of works correctly
        TEST
        (   // string::of(int)
            int Number = 12345;
            EXPECT_EQUAL(string::of(Number), "12345");
        );

        TEST
        (   // string::of(float)
            float Number = 1234.56;
            EXPECT_EQUAL(string::of(Number), "1234.56");
        );

        TEST
        (   // string::of(string)
            // TODO: it'd be nice to test that this doesn't over-allocate
            // (just moves the string), but that's hard to do.
            EXPECT_EQUAL(string::of(string("hi, world")), "hi, world");
        );
    );

    TEST
    (   // append works correctly
        TEST
        (   // append() and += works for rune correctly
            TEST
            (   // On initially empty String...
                {   string String;
                    EXPECT_EQUAL(String.count(), 0);
                    String.append(223);
                    EXPECT_EQUAL(String, "ß");
                    EXPECT_EQUAL(String.count(), 1);
                }

                {   string String;
                    EXPECT_EQUAL(String.count(), 0);
                    String.append(127820);
                    EXPECT_EQUAL(String, "🍌");
                    EXPECT_EQUAL(String.count(), 1);
                }

                {   string String;
                    EXPECT_EQUAL(String.count(), 0);
                    String.append('a');
                    EXPECT_EQUAL(String, "a");
                    EXPECT_EQUAL(String.count(), 1);
                }
            );

            TEST
            (   // On initially non-empty String...
                string String("asdf");
                EXPECT_EQUAL(String.count(), 4);

                String.append(223);
                EXPECT_EQUAL(String, "asdfß");
                EXPECT_EQUAL(String.count(), 5);

                String += rune(127820);
                EXPECT_EQUAL(String, "asdfß🍌");
                EXPECT_EQUAL(String.count(), 6);

                String.append('a');
                EXPECT_EQUAL(String, "asdfß🍌a");
                EXPECT_EQUAL(String.count(), 7);
            );
        );

        TEST
        (   // append() and += works for string correctly
            TEST
            (   // On initially empty String...
                {   string String;
                    EXPECT_EQUAL(String.count(), 0);
                    String += string("hello");
                    EXPECT_EQUAL(String, "hello");
                    EXPECT_EQUAL(String.count(), 5);
                }

                {   string String;
                    EXPECT_EQUAL(String.count(), 0);
                    String += string("🍌");
                    EXPECT_EQUAL(String, "🍌");
                    EXPECT_EQUAL(String.count(), 1);
                }
            );

            TEST
            (   // On initially non-empty String...
                string String("hello");
                EXPECT_EQUAL(String.count(), 5);

                String += ", 🍌world!";
                EXPECT_EQUAL(String, "hello, 🍌world!");
                EXPECT_EQUAL(String.count(), 14);
            );
        );
    );

    TEST
    (   // * and *= work correctly
        TEST
        (   // * works correctly
            string Source("hey");

            EXPECT_EQUAL(Source * 4.4, "heyheyheyhey");
            EXPECT_EQUAL(Source * 3, "heyheyhey");
            EXPECT_EQUAL(Source * 2, "heyhey");
            EXPECT_EQUAL(Source * 1, "hey");
            EXPECT_EQUAL(Source * 0, "");
            // negative numbers reset to blank state
            // TODO: maybe we should reverse the string for negative numbers, to be weird!
            EXPECT_EQUAL(Source * -5, "");

            // does not modify original string
            EXPECT_EQUAL(Source, "hey");
        );

        TEST
        (   // *= works correctly
            string Multiply("hello world");
            Multiply *= 5;
            EXPECT_EQUAL(Multiply, "hello worldhello worldhello worldhello worldhello world");
            Multiply *= 0;
            EXPECT_EQUAL(Multiply, "");
            // reset
            Multiply = "hello world";

            Multiply *= 1;
            EXPECT_EQUAL(Multiply, "hello world");
            // negative numbers reset to blank state
            Multiply *= -1;
            EXPECT_EQUAL(Multiply, "");
        );
    );

    TEST
    (   // () and [] work correctly
        TEST
        (   // [] or () with internal string or stringView
            string External("hey");
            string InternalString("🍌Straße");
            stringView InternalView = InternalString;

            EXPECT_EQUAL(External[InternalString], "hey[🍌Straße]");
            EXPECT_EQUAL(External(InternalString), "hey(🍌Straße)");
            EXPECT_EQUAL(External[InternalView], "hey[🍌Straße]");
            EXPECT_EQUAL(External(InternalView), "hey(🍌Straße)");

            InternalView.pop();
            InternalView.shift();

            EXPECT_EQUAL(External[InternalView], "hey[Straß]");
            EXPECT_EQUAL(External(InternalView), "hey(Straß)");

            // does not modify original string
            EXPECT_EQUAL(InternalString, "🍌Straße");
            EXPECT_EQUAL(External, "hey");
        );

        TEST
        (   // [] or () with external string or stringView
            string ExternalString("Straße🍌");
            stringView ExternalView = ExternalString;
            string Internal("world");

            EXPECT_EQUAL(ExternalString[Internal], "Straße🍌[world]");
            EXPECT_EQUAL(ExternalString(Internal), "Straße🍌(world)");
            EXPECT_EQUAL(ExternalView[Internal], "Straße🍌[world]");
            EXPECT_EQUAL(ExternalView(Internal), "Straße🍌(world)");

            ExternalView.pop();
            ExternalView.shift();

            EXPECT_EQUAL(ExternalView[Internal], "traße[world]");
            EXPECT_EQUAL(ExternalView(Internal), "traße(world)");

            // does not modify original string
            EXPECT_EQUAL(ExternalString, "Straße🍌");
            EXPECT_EQUAL(Internal, "world");
        );
    );

    TEST
    (   // pop() works correctly
        TEST
        (   // On initially non-empty String...
            string String("asdfß🍌a");
            EXPECT_EQUAL(String.count(), 7);

            EXPECT_EQUAL(String.pop(), 'a');
            EXPECT_EQUAL(String, "asdfß🍌");
            EXPECT_EQUAL(String.count(), 6);

            EXPECT_EQUAL(String.pop(), 127820);
            EXPECT_EQUAL(String, "asdfß");
            EXPECT_EQUAL(String.count(), 5);

            EXPECT_EQUAL(String.pop(), 223);
            EXPECT_EQUAL(String, "asdf");
            EXPECT_EQUAL(String.count(), 4);
        );

        TEST
        (   // On single character String
            for (int Shift = 0; Shift <= 20; ++Shift)
            for (int I = 5; I < 30; ++I)
            {   rune Rune = (1 << Shift) + I;
                string String(Rune);
                EXPECT_EQUAL(String.count(), 1);
                EXPECT_EQUAL(String.pop(), Rune);
                EXPECT_EQUAL(String.count(), 0);
            }
        );
    );

    TEST(
        // string::count() works as expected
        string String;
        EXPECT_EQUAL(String.count(), 0);

        String = "hello";
        EXPECT_EQUAL(String.count(), 5);

        String = "ß水𝄋🍌";
        EXPECT_EQUAL(String.count(), 4);
    );

    TEST
    (   // Iteration:
        array<rune> Runes;
        string String("asdFß水 5𝄋7🍌");
        for (const rune &Rune : String.runes())
        {   Runes.append(Rune);
        }
        EXPECT_EQUAL
        (   Runes,
            array<rune>
            ({  'a',
                's',
                'd',
                'F',
                223, // "ß"
                27700, // "水"
                ' ',
                '5',
                119051, // "𝄋"
                '7',
                127820, // "🍌"
            })
        );
    );

    TEST
    (   // sorting/ordering works
        TEST
        (   // operator < works:
            // Examples from https://en.cppreference.com/w/cpp/locale/collate/compare
            string Hrnec("hrnec");
            string Chrt("chrt");
            string Ar("år");
            string Angel("ängel");

            // TODO: not sure why these don't work, maybe because they're not installed on my system.
            // Czech:
            // string::locale("cs_CZ.utf8");
            // EXPECT_EQUAL(Chrt < Hrnec, False);

            // Swedish:
            // string::locale("sv_SE.utf8");
            // EXPECT_EQUAL(Angel < Ar, False);

            string::locale("en_US.utf8");
            EXPECT_EQUAL(Chrt < Hrnec, True);
            EXPECT_EQUAL(Angel < Ar, True);
        );

        TEST
        (   // sorting a string array works
            array<string> Array({"zoo", "apple", "beak", "Astrology", "eagle", "år", "best", "East"});
            Array.sort();
            EXPECT_EQUAL
            (   Array,
                array<string>({"apple", "år", "Astrology", "beak", "best", "eagle", "East", "zoo"})
            );
        );

        TEST
        (   // sorting with a custom comparator works
            stringAsciiCompare compare;

            // "år" should come before "asdf" with utf8 sorting, but not Ascii sorting:
            EXPECT_EQUAL(string("år") < string("asdf"), True);
            EXPECT_EQUAL(compare(string("år"), string("asdf")), False);
            EXPECT_EQUAL(compare(string("asdf"), string("år")), True);

            EXPECT_EQUAL(compare(string("beta"), string("betatest")), True);

            // UTF8 sort will put alpha before Beta:
            EXPECT_EQUAL(string("Beta") < string("alpha"), False);
            // But ASCII cares about case quite a bit:
            EXPECT_EQUAL(compare(string("Beta"), string("alpha")), True);

            EXPECT_EQUAL(compare(string("zoo"), string("zo_")), False);
        );
    );

    TEST
    (   // contains() works
        TEST
        (   // contains(const char *Substring)
            string String("asdf🍌1234hello567world !");
            EXPECT_EQUAL(String.contains("hello"), True);
            EXPECT_EQUAL(String.contains("rld !"), True);
            EXPECT_EQUAL(String.contains("🍌1234"), True);
            EXPECT_EQUAL(String.contains("ß"), False);
            EXPECT_EQUAL(String.contains("4567"), False);
            EXPECT_EQUAL(String.contains("helloworld"), False);
        );

        TEST
        (   // contains(const string &Substring)
            string String("asdf🍌1234hello567world !");
            EXPECT_EQUAL(String.contains(string("hello")), True);
            EXPECT_EQUAL(String.contains(string("rld !")), True);
            EXPECT_EQUAL(String.contains(string("🍌1234")), True);
            EXPECT_EQUAL(String.contains(string("ß")), False);
            EXPECT_EQUAL(String.contains(string("4567")), False);
            EXPECT_EQUAL(String.contains(string("helloworld")), False);
        );
    );

    TEST
    (   // stringView
        TEST
        (   // shift() and empty() work: 
            string String("suß🍌");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView, "suß🍌");
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.shift(), 's');
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.shift(), 'u');
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.shift(), 223);
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.shift(), 127820);
            EXPECT_EQUAL(StringView.empty(), True);
            EXPECT_EQUAL(StringView, "");
            // Initial string should not change:
            EXPECT_EQUAL(String, "suß🍌");
        );

        TEST
        (   // pop() and empty() work: 
            string String("suß🍌");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView, "suß🍌");
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.pop(), 127820);
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.pop(), 223);
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.pop(), 'u');
            EXPECT_EQUAL(StringView.empty(), False);
            EXPECT_EQUAL(StringView.pop(), 's');
            EXPECT_EQUAL(StringView.empty(), True);
            EXPECT_EQUAL(StringView, "");
            // Initial string should not change:
            EXPECT_EQUAL(String, "suß🍌");
        );

        TEST
        (   // first() works: 
            string String("suß🍌");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView.first(), 's');
            // first() doesn't change the string:
            EXPECT_EQUAL(StringView, "suß🍌");

            String = "🍌";
            // first() works for utf8:
            EXPECT_EQUAL(StringView.first(), 127820);
            EXPECT_EQUAL(StringView, "🍌");
        );

        TEST
        (   // last() works: 
            string String("suß🍌");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView.last(), 127820);
            // last() doesn't change the string:
            EXPECT_EQUAL(StringView, "suß🍌");

            String = "asdfghjkl;";
            StringView = String.view(); // reset StringView since String changed
            // last() works for ASCII:
            EXPECT_EQUAL(StringView.last(), ';');
            EXPECT_EQUAL(StringView, "asdfghjkl;");
        );

        TEST
        (   // getting a stringView to print
            string String("Cerberus");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView, String);
            EXPECT_EQUAL(String, StringView);
            std::cout << StringView;
            EXPECT_EQUAL(TestPrintOutput.pull(), "Cerberus");
        );

        TEST
        (   // changing the underlying string does not change the stringView
            // as long as the string size does not increase.
            string String("can");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView, String);
            EXPECT_EQUAL(String, StringView);

            String = "ban";

            // Do not update StringView itself here:
            EXPECT_EQUAL(StringView, String);
            EXPECT_EQUAL(String, StringView);

            std::cout << StringView;
            EXPECT_EQUAL(TestPrintOutput.pull(), "ban");

            String = "sh"; // something shorter

            // Do not update StringView itself here:
            EXPECT_EQUAL(StringView, String);
            EXPECT_EQUAL(String, StringView);

            std::cout << StringView;
            EXPECT_EQUAL(TestPrintOutput.pull(), "sh");
        );

        TEST
        (   // increasing the size of the underlying string does indirectly affect stringView
            string String("tank");
            stringView StringView = String.view();
            EXPECT_EQUAL(StringView, String);
            EXPECT_EQUAL(String, StringView);

            String = "forest";

            // Do not update StringView itself here:
            EXPECT_NOT_EQUAL(StringView, String);
            EXPECT_NOT_EQUAL(String, StringView);

            std::cout << StringView;
            EXPECT_EQUAL(TestPrintOutput.pull(), "fore");
        );

        TEST
        (   // stripping whitespace works ok
            TEST
            (   // no whitespace that it should consume
                TEST
                (   // strip works ok if already no whitespace

                    // temporary stringView:
                    string String("asdf");
                    stringView View = String.view().strip();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asdf");
                    
                    // non-temp stringView:
                    View = String.view();
                    View.strip();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asdf");
                );

                TEST
                (   // stripFront works ok if already no whitespace

                    // temporary stringView:
                    string String("asdf  ");
                    stringView View = String.view().stripFront();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asdf  ");
                    
                    // non-temp stringView:
                    View = String.view();
                    View.stripFront();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asdf  ");
                );

                TEST
                (   // stripBack works ok if already no whitespace

                    // temporary stringView:
                    string String("  asdf");
                    stringView View = String.view().stripBack();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "  asdf");
                    
                    // non-temp stringView:
                    View = String.view();
                    View.stripBack();
                    EXPECT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "  asdf");
                );
            );

            TEST
            (   // should consume whitespace
                TEST
                (   // strip works ok 

                    // temporary stringView:
                    string String("  as  df     ");
                    stringView View = String.view().strip();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "as  df");
                    
                    // non-temp stringView:
                    View = String.view();
                    EXPECT_EQUAL(String, View);
                    View.strip();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "as  df");
                );

                TEST
                (   // stripFront works ok

                    // temporary stringView:
                    string String("        asd f ");
                    stringView View = String.view().stripFront();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asd f ");
                    
                    // non-temp stringView:
                    View = String.view();
                    EXPECT_EQUAL(String, View);
                    View.stripFront();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "asd f ");
                );

                TEST
                (   // stripBack works ok

                    // temporary stringView:
                    string String("   a sdf       ");
                    stringView View = String.view().stripBack();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "   a sdf");
                    
                    // non-temp stringView:
                    View = String.view();
                    EXPECT_EQUAL(String, View);
                    View.stripBack();
                    EXPECT_NOT_EQUAL(String, View);
                    EXPECT_EQUAL(View, "   a sdf");
                );
            );

            TEST
            (   // should consume whitespace with non-ascii around
                // temporary stringView:
                string String("      🍌 as dfß     ");
                stringView View = String.view().strip();
                EXPECT_NOT_EQUAL(String, View);
                EXPECT_EQUAL(View, "🍌 as dfß");
                
                // non-temp stringView:
                View = String.view();
                EXPECT_EQUAL(String, View);
                View.strip();
                EXPECT_NOT_EQUAL(String, View);
                EXPECT_EQUAL(View, "🍌 as dfß");
            );
        );

        TEST
        (   // split
            TEST
            (   // string starting and ending with the delimiter
                string String(":a:b:cd:e:");
                iterator<stringView> SplitIterator = String.view().split(':');
                EXPECT_EQUAL(*SplitIterator.next(), "");
                EXPECT_EQUAL(*SplitIterator.next(), "a");
                EXPECT_EQUAL(*SplitIterator.next(), "b");
                EXPECT_EQUAL(*SplitIterator.next(), "cd");
                EXPECT_EQUAL(*SplitIterator.next(), "e");
                EXPECT_EQUAL(*SplitIterator.next(), "");
                ASSERT(SplitIterator.next() == Null);
            );

            TEST
            (   // ok for empty string
                string String("");
                array<stringView> SplitArray = String.view().split('q');
                EXPECT_EQUAL(SplitArray, array<const char *>({""}));
            );

            TEST
            (   // ok for no delimiters
                string String("🍌asdfßhjkl");
                array<stringView> SplitArray = String.view().split(27700);
                EXPECT_EQUAL(SplitArray, array<const char *>({"🍌asdfßhjkl"}));
            );

            TEST
            (   // ok for just delimiters
                string String("水水水");
                array<stringView> SplitArray = String.view().split(27700);
                EXPECT_EQUAL(SplitArray, array<const char *>({"", "", "", ""}));
            );

            TEST
            (   // string not starting or ending with the delimiter
                string String("a bcde f gh");
                array<stringView> SplitArray = String.view().split(' ');
                EXPECT_EQUAL(SplitArray, array<const char *>({"a", "bcde", "f", "gh"}));
            );

            TEST
            (   // multiple delimiters in a row, also non-ascii delimiter:
                string String("🍌🍌abc🍌🍌d🍌e🍌🍌🍌");
                array<stringView> SplitArray = String.view().split(127820);
                EXPECT_EQUAL
                (   SplitArray,
                    array<const char *>({"", "", "abc", "", "d", "e", "", "", ""})
                );
            );

            TEST
            (   // delimiter is 0
                string String; // can't build this up based on `const char *` array
                               // since c++ string will only copy up to first zero byte.
                String.append(0);
                String.append('a');
                String.append(0);
                String.append('b');
                String.append(0);
                String.append(0);
                String.append(127820);
                String.append(0);
                array<stringView> SplitArray = String.view().split(0);
                EXPECT_EQUAL(SplitArray, array<const char *>({"", "a", "b", "", "🍌", ""}));
            );
        );

        TEST
        (   // integer manipulation methods
            TEST
            (   // stringView::shiftInteger() works as advertized
                TEST
                (   // returns false if no numeric values start the string,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = " 123";
                        stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, " 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "a123";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "水123";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns false if no numeric values start the string, but prefix was + or -,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = "- 123";
                        stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "- 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "+a123";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "-水123";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string,
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "12345";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, 12345);
                    );

                    TEST
                    (   // ends with ASCII
                        string String = "123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, " a");
                        EXPECT_EQUAL(Number, 123);
                    );

                    TEST
                    (   // ends with utf8 runes
                        string String = "56789水";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "水");
                        EXPECT_EQUAL(Number, 56789);
                    );

                    TEST
                    (   // string is completely a binary number:
                        string String = "0b101001";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, 0b101001);
                    );

                    TEST
                    (   // string starts with a binary number:
                        string String = "0b1102";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "2");
                        ASSERT(!StringView.empty());
                        EXPECT_EQUAL(Number, 0b110);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a - prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "-12345";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, -12345);
                    );

                    TEST
                    (   // ends with ASCII
                        string String = "-123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, " a");
                        EXPECT_EQUAL(Number, -123);
                    );

                    TEST
                    (   // ends with utf8 runes
                        string String = "-56789水";
                        stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "水");
                        EXPECT_EQUAL(Number, -56789);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a + prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "+12345";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, 12345);
                    );

                    TEST
                    (   // ends with ASCII
                        string String = "+123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, " a");
                        EXPECT_EQUAL(Number, 123);
                    );

                    TEST
                    (   // ends with utf8 runes
                        string String = "+56789水";
                        stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.shiftInteger(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "水");
                        EXPECT_EQUAL(Number, 56789);
                    );
                );
            );

            TEST
            (   // stringView::integer() for non-const stringView works as advertized
                TEST
                (   // returns false if no numeric values start the string,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = " 123";
                        stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, " 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "a123";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "水123";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns false if no numeric values start the string, but prefix was + or -,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = "- 123";
                        stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "- 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "+a123";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "-水123";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value was the full string
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "1234567890";
                        stringView StringView = String.view();
                        i64 Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, 1234567890);
                    );

                    TEST
                    (   // string started numeric but ended with ASCII
                        string String = "123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // string started numeric but ended with utf8 runes
                        string String = "56789水";
                        stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a - prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "-12345";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, -12345);
                    );

                    TEST
                    (   // but NOT if it ends with ASCII
                        string String = "-123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // but NOT if it ends with utf8 runes
                        string String = "-56789水";
                        stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a + prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "+12345";
                        stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "");
                        ASSERT(StringView.empty());
                        EXPECT_EQUAL(Number, 12345);
                    );

                    TEST
                    (   // but NOT if it ends with ASCII
                        string String = "+123 a";
                        stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // but NOT if it ends with utf8 runes
                        string String = "+56789水";
                        stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );
            );

            TEST
            (   // stringView::integer() for const stringView works as advertized
                TEST
                (   // returns false if no numeric values start the string,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = " 123";
                        const stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, " 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "a123";
                        const stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "水123";
                        const stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns false if no numeric values start the string, but prefix was + or -,
                    // without changing the passed in number.
                    TEST
                    (   // spaces don't count as numeric:
                        string String = "- 123";
                        const stringView StringView = String.view();
                        int Number = -1234;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "- 123");
                        EXPECT_EQUAL(Number, -1234);
                    );

                    TEST
                    (   // other ASCII doesn't count 
                        string String = "+a123";
                        const stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+a123");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // utf8 runes don't count
                        string String = "-水123";
                        const stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-水123");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value was the full string
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "1234567890";
                        const stringView StringView = String.view();
                        i64 Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "1234567890");
                        ASSERT(!StringView.empty());
                        EXPECT_EQUAL(Number, 1234567890);
                    );

                    TEST
                    (   // string started numeric but ended with ASCII
                        string String = "123 a";
                        const stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // string started numeric but ended with utf8 runes
                        string String = "56789水";
                        const stringView StringView = String.view();
                        u16 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a - prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "-12345";
                        const stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "-12345");
                        ASSERT(!StringView.empty());
                        EXPECT_EQUAL(Number, -12345);
                    );

                    TEST
                    (   // but NOT if it ends with ASCII
                        string String = "-123 a";
                        const stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // but NOT if it ends with utf8 runes
                        string String = "-56789水";
                        const stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "-56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );

                TEST
                (   // returns true if a numeric value started the string with a + prefix
                    // changing the passed-in number.
                    TEST
                    (   // string is a number completely:
                        string String = "+12345";
                        const stringView StringView = String.view();
                        int Number = 0;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), True);

                        EXPECT_EQUAL(StringView, "+12345");
                        ASSERT(!StringView.empty());
                        EXPECT_EQUAL(Number, 12345);
                    );

                    TEST
                    (   // but NOT if it ends with ASCII
                        string String = "+123 a";
                        const stringView StringView = String.view();
                        i8 Number = -5;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+123 a");
                        EXPECT_EQUAL(Number, -5);
                    );

                    TEST
                    (   // but NOT if it ends with utf8 runes
                        string String = "+56789水";
                        const stringView StringView = String.view();
                        i32 Number = 256;

                        EXPECT_EQUAL(StringView.integer(determining(Number)), False);

                        EXPECT_EQUAL(StringView, "+56789水");
                        EXPECT_EQUAL(Number, 256);
                    );
                );
            );
        );

        TEST
        (   // "real" number manipulation methods
            TEST
            (   // stringView::shiftReal() works as advertized
                TEST
                (   // works for positive integer followed by decimals, nothing else
                    string String = "+1234.5";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), True);

                    EXPECT_EQUAL(Double, 1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(1234.5));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "+1234.5");
                );

                TEST
                (   // works for positive integer followed by trailing period
                    string String = "+128.";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), True);

                    EXPECT_EQUAL(Double, 128);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(128));
                    EXPECT_EQUAL(StringView, ""); // stringView consumes the period
                    EXPECT_EQUAL(String, "+128.");
                );

                TEST
                (   // works for positive integer followed by decimals, plus non-numeric
                    string String = "+123.45.";
                    stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Float)), True);

                    EXPECT_EQUAL(Float, 123.45);
                    // TODO: we could try to change our algorithm to be better here:
                    ASSERT(Float == float(123.4500045776));
                    EXPECT_EQUAL(StringView, ".");
                    EXPECT_EQUAL(String, "+123.45.");
                );

                TEST
                (   // works for negative integer followed by decimals, nothing else
                    string String = "-1234.5";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), True);

                    EXPECT_EQUAL(Double, -1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(-1234.5));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "-1234.5");
                );

                TEST
                (   // works for negative integer followed by trailing period
                    string String = "-128.";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), True);

                    EXPECT_EQUAL(Double, -128);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(-128));
                    EXPECT_EQUAL(StringView, ""); // stringView consumes the period
                    EXPECT_EQUAL(String, "-128.");
                );

                TEST
                (   // works for negative integer followed by decimals, plus non-numeric
                    string String = "-123.45.";
                    stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Float)), True);

                    EXPECT_EQUAL(Float, -123.45);
                    // Ensure that equality is really a given here:
                    // TODO: we could try to change our algorithm to be better here:
                    ASSERT(Float == float(-123.4500045776));
                    EXPECT_EQUAL(StringView, ".");
                    EXPECT_EQUAL(String, "-123.45.");
                );

                TEST
                (   // ensures things are negative for -0.1234 like strings
                    string String = "-0.1234asdf";
                    stringView StringView = String.view();
                    float Float = -204;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Float)), True);

                    EXPECT_EQUAL(Float, -0.1234);
                    // Ensure that equality is really a given here:
                    ASSERT(Float == float(-0.1234));
                    EXPECT_EQUAL(StringView, "asdf");
                    EXPECT_EQUAL(String, "-0.1234asdf");
                );

                TEST
                (   // ensures things are positive for +0.431 like strings
                    string String = "+0.431 543";
                    stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Float)), True);

                    EXPECT_EQUAL(Float, 0.431);
                    ASSERT(Float == float(0.431));
                    EXPECT_EQUAL(StringView, " 543");
                    EXPECT_EQUAL(String, "+0.431 543");
                );

                TEST
                (   // resets stringView if a + or - is found but no subsequent digit
                    string String = "+A";
                    stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Float)), False);

                    EXPECT_EQUAL(Float, -10);
                    EXPECT_EQUAL(StringView, "+A");
                    EXPECT_EQUAL(String, "+A");

                    String = "- ";
                    StringView = String.view();
                    double Double = 20.3;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), False);

                    EXPECT_EQUAL(Double, 20.3);
                    EXPECT_EQUAL(StringView, "- ");
                    EXPECT_EQUAL(String, "- ");
                );

                TEST
                (   // resets stringView if a . is found before any digit
                    string String = ". bork";
                    stringView StringView = String.view();
                    double Double = -123.4;

                    EXPECT_EQUAL(StringView.shiftReal(determining(Double)), False);

                    EXPECT_EQUAL(Double, -123.4);
                    EXPECT_EQUAL(StringView, ". bork");
                    EXPECT_EQUAL(String, ". bork");
                );
            );

            TEST
            (   // stringView::real() & works as advertized
                TEST
                (   // works for positive integer followed by decimals, nothing else
                    string String = "+1234.5";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), True);

                    EXPECT_EQUAL(Double, 1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(1234.5));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "+1234.5");
                );

                TEST
                (   // works for positive integer followed by decimals, plus non-numeric
                    string String = "+123.45.";
                    stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -4);
                    ASSERT(Float == float(-4));
                    EXPECT_EQUAL(StringView, "+123.45.");
                    EXPECT_EQUAL(String, "+123.45.");
                );

                TEST
                (   // works for negative integer followed by decimals, nothing else
                    string String = "-1234.5";
                    stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), True);

                    EXPECT_EQUAL(Double, -1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(-1234.5));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "-1234.5");
                );

                TEST
                (   // works for negative integer followed by decimals, plus non-numeric
                    string String = "-123.45.";
                    stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -4);
                    // Ensure that equality is really a given here:
                    ASSERT(Float == float(-4));
                    EXPECT_EQUAL(StringView, "-123.45.");
                    EXPECT_EQUAL(String, "-123.45.");
                );

                TEST
                (   // ensures things are negative for -0.1234 like strings
                    string String = "-0.1234";
                    stringView StringView = String.view();
                    float Float = -204;

                    EXPECT_EQUAL(StringView.real(determining(Float)), True);

                    EXPECT_EQUAL(Float, -0.1234);
                    // Ensure that equality is really a given here:
                    ASSERT(Float == float(-0.1234));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "-0.1234");
                );

                TEST
                (   // ensures things are positive for +0.431 like strings
                    string String = "+0.431";
                    stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(StringView.real(determining(Float)), True);

                    EXPECT_EQUAL(Float, 0.431);
                    ASSERT(Float == float(0.431));
                    EXPECT_EQUAL(StringView, "");
                    EXPECT_EQUAL(String, "+0.431");
                );

                TEST
                (   // resets stringView if a + or - is found but no subsequent digit
                    string String = "+A";
                    stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(StringView.real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -10);
                    EXPECT_EQUAL(StringView, "+A");
                    EXPECT_EQUAL(String, "+A");

                    String = "- ";
                    StringView = String.view();
                    double Double = 20.3;

                    EXPECT_EQUAL(StringView.real(determining(Double)), False);

                    EXPECT_EQUAL(Double, 20.3);
                    EXPECT_EQUAL(StringView, "- ");
                    EXPECT_EQUAL(String, "- ");
                );

                TEST
                (   // resets stringView if a . is found before any digit
                    string String = ".5";
                    stringView StringView = String.view();
                    double Double = -123.4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), False);

                    EXPECT_EQUAL(Double, -123.4);
                    EXPECT_EQUAL(StringView, ".5");
                    EXPECT_EQUAL(String, ".5");
                );
            );

            TEST
            (   // stringView::real() const & works as advertized
                TEST
                (   // works for positive integer followed by decimals, nothing else
                    string String = "+1234.5";
                    const stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), True);

                    EXPECT_EQUAL(Double, 1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(1234.5));
                    EXPECT_EQUAL(StringView, "+1234.5");
                    EXPECT_EQUAL(String, "+1234.5");
                );

                TEST
                (   // works for positive integer followed by decimals, plus non-numeric
                    string String = "+123.45.";
                    const stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -4);
                    ASSERT(Float == float(-4));
                    EXPECT_EQUAL(StringView, "+123.45.");
                    EXPECT_EQUAL(String, "+123.45.");
                );

                TEST
                (   // works for negative integer followed by decimals, nothing else
                    string String = "-1234.5";
                    const stringView StringView = String.view();
                    double Double = -4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), True);

                    EXPECT_EQUAL(Double, -1234.5);
                    // Ensure that equality is really a given here:
                    ASSERT(Double == double(-1234.5));
                    EXPECT_EQUAL(StringView, "-1234.5");
                    EXPECT_EQUAL(String, "-1234.5");
                );

                TEST
                (   // works for negative integer followed by decimals, plus non-numeric
                    string String = "-123.45.";
                    const stringView StringView = String.view();
                    float Float = -4;

                    EXPECT_EQUAL(StringView.real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -4);
                    // Ensure that equality is really a given here:
                    ASSERT(Float == float(-4));
                    EXPECT_EQUAL(StringView, "-123.45.");
                    EXPECT_EQUAL(String, "-123.45.");
                );

                TEST
                (   // ensures things are negative for -0.1234 like strings
                    string String = "-0.1234";
                    const stringView StringView = String.view();
                    float Float = -204;

                    EXPECT_EQUAL(StringView.real(determining(Float)), True);

                    EXPECT_EQUAL(Float, -0.1234);
                    // Ensure that equality is really a given here:
                    ASSERT(Float == float(-0.1234));
                    EXPECT_EQUAL(StringView, "-0.1234");
                    EXPECT_EQUAL(String, "-0.1234");
                );

                TEST
                (   // ensures things are positive for +0.431 like strings
                    string String = "+0.431";
                    const stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(StringView.real(determining(Float)), True);

                    EXPECT_EQUAL(Float, 0.431);
                    ASSERT(Float == float(0.431));
                    EXPECT_EQUAL(StringView, "+0.431");
                    EXPECT_EQUAL(String, "+0.431");
                );

                TEST
                (   // resets stringView if a + or - is found but no subsequent digit
                    string String = "+A";
                    stringView StringView = String.view();
                    float Float = -10;

                    EXPECT_EQUAL(constant(StringView).real(determining(Float)), False);

                    EXPECT_EQUAL(Float, -10);
                    EXPECT_EQUAL(StringView, "+A");
                    EXPECT_EQUAL(String, "+A");

                    String = "- ";
                    StringView = String.view();
                    double Double = 20.3;

                    EXPECT_EQUAL(constant(StringView).real(determining(Double)), False);

                    EXPECT_EQUAL(Double, 20.3);
                    EXPECT_EQUAL(StringView, "- ");
                    EXPECT_EQUAL(String, "- ");
                );

                TEST
                (   // resets stringView if a . is found before any digit
                    string String = ".5";
                    stringView StringView = String.view();
                    double Double = -123.4;

                    EXPECT_EQUAL(StringView.real(determining(Double)), False);

                    EXPECT_EQUAL(Double, -123.4);
                    EXPECT_EQUAL(StringView, ".5");
                    EXPECT_EQUAL(String, ".5");
                );

                TEST
                (   // works for edge cases like +0.5, 0.5, -0.5, 1., 2.0
                    float Float = -10;

                    EXPECT_EQUAL(string("+0.5").view().real(determining(Float)), True);

                    ASSERT(Float == float(0.5));

                    EXPECT_EQUAL(string("-0.5").view().real(determining(Float)), True);

                    ASSERT(Float == float(-0.5));

                    EXPECT_EQUAL(string("0.5").view().real(determining(Float)), True);

                    ASSERT(Float == float(0.5));

                    EXPECT_EQUAL(string("1.").view().real(determining(Float)), True);

                    ASSERT(Float == float(1));

                    EXPECT_EQUAL(string("2.0").view().real(determining(Float)), True);

                    ASSERT(Float == float(2));
                );
            );
        );
    );

    // TODO: string + string doesn't affect other string
    
    /* TODO
    TEST(
        // Cropping when text will be cropped
        TEST(
            // ASCII only with overflow
            String string("hey there peeps!");
            Array<String> split;
            for (String line : string.splitOnWidth(3)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>({
                "hey",
                " th",
                "ere",
                " pe",
                "eps",
                "!",
            }));
        );

        TEST(
            // ASCII only with exact word boundary
            String string("Good Day");
            Array<String> split;
            for (String line : string.splitOnWidth(4)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>({
                "Good",
                " Day",
            }));
        );

        TEST(
            // Splitting non-ASCII character ok:
            String string = "水1水23水";
            Array<String> split;
            for (String line : string.splitOnWidth(2)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>({
                "水",
                "1",
                "水",
                "23",
                "水",
            }));
        );

        TEST(
            // non-ASCII with overflow
            String string = "rsß水a🍌";
            Array<String> split;
            for (String line : string.splitOnWidth(4)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>({
                "rsß", // not four letters long
                "水a", // not four letters either
                "🍌",
            }));
        );

        TEST(
            // ASCII and non-ASCII with exact word boundary
            String string = "asß水5123";
            Array<String> split;
            for (String line : string.splitOnWidth(3)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>({
                "asß",
                "水5",
                "123",
            }));
        );

        TEST(
            // Splitting when a character is too big for the line:
            String string = "水1234";
            Array<String> split;
            for (String line : string.splitOnWidth(1)) {
                split.append(line);
            }
            EXPECT_EQUAL(split, Array<String>());
        );
    );

    TEST(
        // Cropping when text will not be cropped
        TEST(
            // ASCII
            for (Int i = 8; i < 12; ++i) {
                String string("hey THIS");
                Array<String> split;
                for (String line : string.splitOnWidth(i)) {
                    split.append(line);
                }
                EXPECT_EQUAL(split, Array<String>({"hey THIS"}));
            }
        );

        TEST(
            // Mix of ASCII with non-ASCII:
            for (Int i = 7; i < 15; ++i) {
                String string = "水1234ß";
                Array<String> split;
                for (String line : string.splitOnWidth(i)) {
                    split.append(line);
                }
                EXPECT_EQUAL(split, Array<String>({"水1234ß"}));
            }
        );
    );
    */
}
#endif

TMVB

std::size_t std::hash<bvmt::string>::operator() (const bvmt::string& String) const {
    return hash<std::string>()(String.Internal);
}
