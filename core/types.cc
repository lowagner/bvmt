#include "types.h"

#ifndef NDEBUG
#include "error.h"
#include "string.h"
#endif

BVMT

template <>
const char *type<i64>::Name = "i64";

char nibble(u8 Value) {
    static const char Hex[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
    };
    ASSERT(Value < 16);
    return Value < 16 ? Hex[Value] : '?';
}

std::ostream &operator << (std::ostream &Out, const u8 &U8) {
    return Out << "u8(" << (int)U8 << ")";
}

std::ostream &operator << (std::ostream &Out, const i8 &I8) {
    return Out << "i8(" << (int)I8 << ")";
}

#ifndef NDEBUG
namespace test {
    parent::parent(i64 V) : noisy(V, "parent") {}

    parent::parent(const parent &Other)
    :   noisy(Other.Value, "parent")
    {   std::cout << "{CC}";
    }

    parent::parent(parent &&Other)
    :   noisy(Other.Value, "parent")
    {   Other.Value *= -1;
        std::cout << "{MC}";
    }

    parent &parent::operator = (parent &&Other) {
        SET_EQUAL_GUARD(
            Other,
            Value = Other.Value;
            Other.Value *= -1;
            print();
            std::cout << "{MA}";
        );
    }

    parent &parent::operator = (const parent &Other) {
        SET_EQUAL_GUARD(
            Other,
            Value = Other.Value;
            std::cout << "{CA}";
        );
    }
    
    parent::~parent() {}

    bool parent::equals(const noisy &Other) const {
        if (const parent *Parent = dynamic_cast<const parent*>(&Other)) {
            return Value == Parent->Value;
        }
        ASSERT(False /* Should never get here based on noisy. */);
        return False;
    }

    void parent::print(std::ostream &Out) const {
        Out << "parent(" << Value << ")";
    }

    child::child(std::string N, i64 V)
    :   parent(V), Name(N)
    {   print();
    }

    child::child(const child &Other)
    :   parent(Other.Value), Name(Other.Name)
    {   print();
        std::cout << "{CC}";
    }

    child::child(child &&Other)
    :   parent(Other.Value), Name(std::move(Other.Name))
    {   print();
        Other.Value *= -1;
        Other.Name = "";
        std::cout << "{CC}";
    }

    void child::copyOver(const child& From) {
        parent::copyOver(From);
        this->Name = From.Name;
    }
    
    void child::print(std::ostream &Out) const {
        Out << "child(" << this->Name << ", " << this->Value << ")";
    }

    child::~child() {
        std::cout << "~";
        print();
    }

    bool child::equals(const noisy &Other) const {
        if (const child *Child = dynamic_cast<const child*>(&Other))
        {   return Value == Child->Value && Name == Child->Name;
        }
        ASSERT(False /* Should never get here based on noisy. */);
        return False;
    }

    aunt::aunt(std::string N) : noisy(0), Name(N) {}

    aunt::aunt(const child &Child)
    :   noisy(0, Null), Name(Child.Name + "'s aunt")
    {   print();
        std::cout << "{CC[child]}";
    }

    void aunt::copyOver(const aunt& From) {
        Name = From.Name;
    }

    void aunt::print(std::ostream &Out) const {
        Out << "aunt(" << Name << ")";
    }

    aunt::~aunt() {
        std::cout << "~";
        print();
    }
}

template <>
bool checkEqual(const test::child &, const test::aunt &) {
    return False;
}

template <>
bool checkEqual(const test::aunt &, const test::child &) {
    return False;
}

template <>
bool checkEqual(const test::aunt &A, const test::aunt &B) {
    return checkEqual(A.Name, B.Name);
}

template <>
bool checkEqual(const test::parent &, const test::aunt &) {
    return False;
}

template <>
bool checkEqual(const test::aunt &, const test::parent &) {
    return False;
}

namespace test {
    noisy::noisy(i64 V, const char *T)
    :   TypeName(T), Value(V)
    {   print();
    }

    void noisy::copyOver(const noisy &From) {
        TypeName = From.TypeName;
        Value = From.Value;
    }

    void noisy::print(std::ostream &Out) const {
        if (TypeName != Null) {
            Out << TypeName << "(" << Value << ")";
        }
    }

    noisy::noisy(const noisy &N) {
        copyOver(N);
        print();
        std::cout << "{CC}";
    }

    noisy &noisy::operator = (const noisy &N) {
        if (this != &N) {
            copyOver(N);
            print();
            std::cout << "{CA}";
        } else {
            print();
            std::cout << "[oops, copy-assigned to self]";
        }
        return This;
    }

    noisy::noisy(noisy &&N) {
        copyOver(N);
        print();
        N.Value *= -1; // Modifying the old moved instance, for clarity in tests.
        std::cout << "{MC}";
    }

    noisy &noisy::operator = (noisy &&N) {
        if (this != &N) {
            copyOver(N);
            print();
            std::cout << "{MA}";
            N.Value *= -1; // Modifying the old moved instance, for clarity in tests.
        } else {
            print();
            std::cout << "[oops, move-assigned to self]";
        }
        return *this;
    }

    noisy::~noisy() {
        if (TypeName != Null) std::cout << "~";
        print();
    }

    bool noisy::equals(const noisy &Other) const {
        return Value == Other.Value;
    }

    bool operator == (const noisy &N1, const noisy &N2) {
        if (typeid(N1) == typeid(N2)) {
            return N1.equals(N2);
        }
        return False;
    }

    std::ostream &operator <<(std::ostream &Out, const noisy &N) {
        N.print(Out);
        return Out;
    }
}

// test struct we don't use anywhere else.
struct twoRefs {
    u8 &Ref1;
    u8 &Ref2;
    twoRefs(u8 &R1, u8 &R2) : Ref1(R1), Ref2(R2) {}
};

void test__core__types() {
    TEST(
        "noisyU8s",
        using test::noisyU8s;
        TEST(
            "have the correct size",
            EXPECT_EQUAL(sizeof(noisyU8s<3>), 3u);
            EXPECT_EQUAL(sizeof(noisyU8s<1>), 1u);
            EXPECT_EQUAL(sizeof(noisyU8s<8>), 8u);
        );

        TEST(
            "equality works correctly",
            EXPECT_EQUAL(noisyU8s<3>(5), noisyU8s<3>(5));
            EXPECT_NOT_EQUAL(noisyU8s<3>(5), noisyU8s<3>(-1));
        );

        TEST(
            "destructor works correctly",
            for (int I = 0; I < 1000; I += 47) {
                {
                    noisyU8s<2> U16(I);
                    EXPECT_EQUAL(TestPrintOutput.pull(), string("noisy")(string::of(I)));
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), string("~noisy")(string::of(I)));
            }
        );

        TEST(
            "copy constructor works correctly",
            {   
                noisyU8s<2> Source(100);
                {   
                    noisyU8s<2> Destination;
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction
                   
                    Destination = Source;

                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100){CA}");
                    Destination.value(200);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(200)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(100)");
        );

        TEST(
            "copy assignment works correctly",
            {   
                noisyU8s<2> Source(100);
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction
                {   
                    noisyU8s<2> Destination(Source);

                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100){CC}");
                    Destination.value(200);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(200)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(100)");
        );

        TEST(
            "move constructor works correctly",
            {   
                noisyU8s<2> Source(100);
                {   
                    noisyU8s<2> Destination;
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction
                   
                    Destination = std::move(Source);

                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100){MA}");
                    Destination.value(200);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(200)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(-100)");
        );

        TEST(
            "move assignment works correctly",
            {   
                noisyU8s<2> Source(100);
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction
                {   
                    noisyU8s<2> Destination(std::move(Source));

                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100){MC}");
                    Destination.value(200);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(200)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(-100)");
        );
    );

    TEST(
        "equality between parent/child/noisy",
        EXPECT_EQUAL(type<test::parent>$$ equals<test::noisy>(), False);
        EXPECT_EQUAL(type<test::child>$$ equals<test::child>(), True);
    );

    TEST(
        "extends works correctly",
        EXPECT_EQUAL(type<test::parent>$$ extends<test::noisy>(), True);
        EXPECT_EQUAL(type<test::noisy>$$ extends<test::parent>(), False);

        EXPECT_EQUAL(type<test::child>$$ extends<test::parent>(), True);
        EXPECT_EQUAL(type<test::parent>$$ extends<test::child>(), False);

        EXPECT_EQUAL(type<test::child>$$ extends<test::noisy>(), True);
        EXPECT_EQUAL(type<test::noisy>$$ extends<test::child>(), False);

        EXPECT_EQUAL(type<test::child>$$ extends<test::child>(), True);
    );

    TEST(
        "clamp works correctly",
        TEST(
            "i8 clamping",
            // below
            EXPECT_EQUAL(type<i8>::clamp(i16(-200)), -128);
            // above
            EXPECT_EQUAL(type<i8>::clamp(i16(128)), 127);
            // inside
            EXPECT_EQUAL(type<i8>::clamp(i16(10)), 10);
            EXPECT_EQUAL(type<i8>::clamp(i16(-10)), -10);
        );

        TEST(
            "i16 clamping",
            // below
            EXPECT_EQUAL(type<i16>::clamp(i32(-200200)), -32768);
            // above
            EXPECT_EQUAL(type<i16>::clamp(i32(100000)), 32767);
            // inside
            EXPECT_EQUAL(type<i16>::clamp(i32(1300)), 1300);
            EXPECT_EQUAL(type<i16>::clamp(i32(-8010)), -8010);
        );
    );

    TEST(
        "checkEqual",
        TEST(
            "floats",
            TEST(
                "tiny floats don't care about sign",
                EXPECT_EQUAL(+1.23456e-8, +1.23456e-8);
                EXPECT_EQUAL(+1.23456e-8, -1.23456e-8);
                EXPECT_EQUAL(-1.23456e-8, +1.23456e-8);
                EXPECT_EQUAL(-1.23456e-8, -1.23456e-8);
            );

            TEST(
                "small floats",
                EXPECT_EQUAL(1.23456e-3, 1.23457e-3);
                EXPECT_NOT_EQUAL(1.2345e-3, 1.23461e-3);
                EXPECT_NOT_EQUAL(1.2345e-3, 1.23439e-3);

                // small (but not tiny) floats do care about sign:
                EXPECT_NOT_EQUAL(+1.234e-3, -1.234e-3);
                EXPECT_NOT_EQUAL(-1.234e-3, +1.234e-3);
                EXPECT_EQUAL(-1.234e-3, -1.234e-3);
            );

            TEST(
                "bigger floats care about approximately 5 digits",
                EXPECT_EQUAL(1.23456, 1.23455);
                EXPECT_EQUAL(1.23456, 1.23456);
                EXPECT_EQUAL(1.23456, 1.23457);
                EXPECT_NOT_EQUAL(1.23456, 1.23454);

                EXPECT_EQUAL(12.3456, 12.3455);
                EXPECT_EQUAL(12.3456, 12.3456);
                EXPECT_EQUAL(12.3456, 12.3457);
                EXPECT_NOT_EQUAL(12.3456, 12.3454);

                EXPECT_EQUAL(123.456, 123.455);
                EXPECT_EQUAL(123.456, 123.456);
                EXPECT_EQUAL(123.456, 123.457);
                EXPECT_NOT_EQUAL(123.456, 123.454);

                EXPECT_EQUAL(1234.56, 1234.55);
                EXPECT_EQUAL(1234.56, 1234.56);
                EXPECT_EQUAL(1234.56, 1234.57);
                EXPECT_NOT_EQUAL(1234.56, 1234.54);

                EXPECT_EQUAL(12345.6, 12345.5);
                EXPECT_EQUAL(12345.6, 12345.6);
                EXPECT_EQUAL(12345.6, 12345.7);
                EXPECT_NOT_EQUAL(12345.6, 12345.4);

                // big floats do care about sign:
                EXPECT_NOT_EQUAL(+7.65432, -7.65432);
                EXPECT_NOT_EQUAL(-7.65432, +7.65432);
                EXPECT_EQUAL(-7.65432, -7.65432);
                EXPECT_EQUAL(+7.65432, +7.65432);

                EXPECT_NOT_EQUAL(+987.65, -987.65);
                EXPECT_NOT_EQUAL(-987.65, +987.65);
                EXPECT_EQUAL(-987.65, -987.65);
                EXPECT_EQUAL(+987.65, +987.65);
            );
        );
    );
}
#endif

TMVB
