#include "error.h"

BVMT

error::error(std::string M, const char *A) throw() : Message(M), At(A) {}
error::error(const char *M, const char *A) throw() : Message(M), At(A) {}
error::error(std::string M, std::string A) throw() : Message(M), At(A) {}
error::error(const char *M, std::string A) throw() : Message(M), At(A) {}

const char* error::what() const throw()
{   return Message.c_str();
}

capturer::capturer(std::ostream &Os)
:   Ostream(Os), Out(), OldBuffer(Ostream.rdbuf(Out.rdbuf()))
{}

capturer::~capturer()
{   stopCapture();
}

std::string capturer::pull()
{   std::string Output = Out.str();
    Out.str("");
    return Output;
}

void capturer::stopCapture()
{   if (OldBuffer)
    {   Ostream.rdbuf(OldBuffer);
        OldBuffer = Null;
    }
}

SINGLETON_CC(debug, {})

#ifndef NDEBUG
template <>
bool checkEqual(const char *const &A, const char *const &B) {
    return strcmp(A, B) == 0;
}
template <>
bool checkEqual(const char *const &A, const std::string &B) {
    return B == A;
}
template <>
bool checkEqual(const std::string &A, const char *const &B) {
    return A == B;
}

void test__core__error()
{   TEST("expect equal works", EXPECT_EQUAL(1, 1));

    TEST("expect throw works", EXPECT_THROW(throw error("testing!", AT), "testing!"));

    TEST("expect throw can fail", EXPECT_THROW(EXPECT_EQUAL(5, 100), "expected 5 to equal 100"));

    TEST
    (   "failing tests throw correctly, with context",
        EXPECT_THROW
        (   TEST
            (   "test should fail",
                EXPECT_EQUAL(True, False);
            ),
            "test should fail: expected True to equal False"
        );
    );

    TEST
    (   "failing tests throw correctly, with context, and can << context in",
        EXPECT_THROW
        (   for (int I = 0; I < 10; ++I)
            {   TEST
                (   "test " << I,
                    bool Value = I == 7 ? False : True;
                    EXPECT_EQUAL(Value, True);
                )
            },
            "test 7: expected Value to equal True"
        );
    );

    TEST
    (   "try will throw correctly, with context",
        EXPECT_THROW
        (   TRY
            (   "try should fail",
                throw error("oops!", AT);
            ),
            "try should fail: oops!"
        );
    );

    TEST
    (   "TEST_TYPE(S)",
        TEST
        (   "TEST_TYPE",
            TEST_TYPE
            (   "testing one type can work fine",
                i64,
                testType X = 1;
                std::cout << X;
                EXPECT_EQUAL(TestPrintOutput.pull(), "1");
            );

            EXPECT_THROW
            (   TEST_TYPE
                (   "testing one type can fail",
                    i8,
                    testType X = -2;
                    ASSERT(X > 0);
                ),
                "testing one type can fail with i8 testType: expected X > 0"
            );
        );

        TEST
        (   "TEST_2_TYPES",
            TEST_2_TYPES
            (   "test two types",
                i64, u64,
                // testing two types can work fine
                testType X = 5;
                std::cout << X;
                EXPECT_EQUAL(TestPrintOutput.pull(), "5");
            );

            EXPECT_THROW
            (   TEST_2_TYPES
                (   "testing two types can fail on first type",
                    i64, u64,
                    testType X = 1L << 63;
                    ASSERT(X > 0);
                ),
                "testing two types can fail on first type with i64 testType: expected X > 0"
            );

            EXPECT_THROW
            (   TEST_2_TYPES
                (   "testing two types can fail on second type",
                    i64, u64,
                    testType X = -1;
                    ASSERT(X < 0); // OK for compiler warning here.
                ),
                "testing two types can fail on second type with u64 testType: expected X < 0"
            );
        );

        TEST
        (   "TEST_3_TYPES",
            TEST_3_TYPES
            (   "test three types",
                i64, u64, i32,
                // testing three types can work fine
                testType X = 5;
                std::cout << X;
                EXPECT_EQUAL(TestPrintOutput.pull(), "5");
            );

            EXPECT_THROW
            (   TEST_3_TYPES
                (   "testing three types can fail on first type",
                    i64, u64, u32,
                    testType X = -1;
                    ASSERT(X > 0);
                ),
                "testing three types can fail on first type with i64 testType: expected X > 0"
            );

            EXPECT_THROW
            (   TEST_3_TYPES
                (   "testing three types can fail on second type",
                    i64, u64, i8,
                    testType X = -1;
                    ASSERT(X < 0); // OK for compiler warning here.
                ),
                "testing three types can fail on second type with u64 testType: expected X < 0"
            );

            EXPECT_THROW
            (   TEST_3_TYPES
                (   "testing three types can fail on third type",
                    i64, u64, i8,
                    testType X = 257; // OK for compiler warning here.
                    ASSERT(X == 257);
                ),
                "testing three types can fail on third type with i8 testType: expected X == 257"
            );
        );

        TEST
        (   "TEST_4_TYPES",
            TEST_4_TYPES
            (   "four types",
                i64, u64, i32, u16,
                // testing four types can work fine
                testType X = 5;
                std::cout << X;
                EXPECT_EQUAL(TestPrintOutput.pull(), "5");
            );

            EXPECT_THROW
            (   TEST_4_TYPES
                (   "testing four types can fail on first type",
                    i64, u64, u32, u8,
                    testType X = -1;
                    ASSERT(X > 0);
                ),
                "testing four types can fail on first type with i64 testType: expected X > 0"
            );

            EXPECT_THROW
            (   TEST_4_TYPES
                (   "testing four types can fail on second type",
                    i64, u64, i8, i16,
                    testType X = -1;
                    ASSERT(X < 0); // OK for compiler warning here.
                ),
                "testing four types can fail on second type with u64 testType: expected X < 0"
            );

            EXPECT_THROW
            (   TEST_4_TYPES
                (   "testing four types can fail on third type",
                    i64, u64, i8, u32,
                    testType X = 257; // OK for compiler warning here.
                    ASSERT(X == 257);
                ),
                "testing four types can fail on third type with i8 testType: expected X == 257"
            );

            EXPECT_THROW
            (   TEST_4_TYPES
                (   "testing four types can fail on fourth type",
                    i64, u64, i8, dbl,
                    ASSERT(type<testType>::IsIntegral);
                ),
                "testing four types can fail on fourth type with dbl testType: expected type<testType>::IsIntegral"
            );
        );
    );

    TEST
    (   "TEST_VALUE(S)",
        TEST
        (   "TEST_VALUE",
            TEST_VALUE
            (   "testing with value",
                101,
                // can work ok
                EXPECT_EQUAL(TestValue % 2, 1);
            );

            EXPECT_THROW
            (   TEST_VALUE
                (   "testing can fail",
                    253,
                    EXPECT_EQUAL(TestValue % 2, 0);
                ),
                "testing can fail with TestValue=253: expected TestValue % 2 to equal 0"
            );
        );

        TEST
        (   "TEST_2_VALUES",
            TEST_2_VALUES
            (   "testing with values can work ok",
                5, 13,
                EXPECT_EQUAL(TestValue % 2, 1);
            );

            EXPECT_THROW
            (   TEST_2_VALUES
                (   "testing two values can fail on first",
                    255, 100,
                    EXPECT_EQUAL(TestValue % 2, 0);
                ),
                "testing two values can fail on first with TestValue=255: expected TestValue % 2 to equal 0"
            );

            EXPECT_THROW
            (   TEST_2_VALUES
                (   "testing two values can fail on second",
                    100, 201,
                    EXPECT_EQUAL(TestValue % 2, 0);
                ),
                "testing two values can fail on second with TestValue=201: expected TestValue % 2 to equal 0"
            );
        );

        TEST
        (   "TEST_3_VALUES",
            TEST_3_VALUES
            (   "testing with values can work ok",
                5, 10, 40,
                int X = 2 * TestValue;
                EXPECT_EQUAL(X, 2 * TestValue);
            );

            EXPECT_THROW
            (   TEST_3_VALUES
                (   "testing three values can fail on first",
                    255, 100, 30,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing three values can fail on first with TestValue=255: expected X to equal 2 * TestValue"
            );

            EXPECT_THROW
            (   TEST_3_VALUES
                (   "testing three values can fail on second",
                    100, 200, 30,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing three values can fail on second with TestValue=200: expected X to equal 2 * TestValue"
            );

            EXPECT_THROW
            (   TEST_3_VALUES
                (   "testing three values can fail on third",
                    100, 10, 129,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing three values can fail on third with TestValue=129: expected X to equal 2 * TestValue"
            );
        );

        TEST
        (   "TEST_4_VALUES",
            TEST_4_VALUES
            (   "four values can work ok",
                5, 10, 40, -1000,
                int X = 2 * TestValue;
                EXPECT_EQUAL(X, 2 * TestValue);
            );

            EXPECT_THROW
            (   TEST_4_VALUES
                (   "testing four values can fail on first",
                    255, 100, 30, 2,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing four values can fail on first with TestValue=255: expected X to equal 2 * TestValue"
            );

            EXPECT_THROW
            (   TEST_4_VALUES
                (   "testing four values can fail on second",
                    100, 200, 30, 2,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing four values can fail on second with TestValue=200: expected X to equal 2 * TestValue"
            );

            EXPECT_THROW
            (   TEST_4_VALUES
                (   "testing four values can fail on third",
                    100, 10, 129, 2,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing four values can fail on third with TestValue=129: expected X to equal 2 * TestValue"
            );

            EXPECT_THROW
            (   TEST_4_VALUES
                (   "testing four values can fail on fourth",
                    103, 104, 127, -2,
                    // oh no, overflow
                    u8 X = 2 * TestValue;
                    EXPECT_EQUAL(X, 2 * TestValue);
                ),
                "testing four values can fail on fourth with TestValue=-2: expected X to equal 2 * TestValue"
            );
        );
    );

    TEST
    (   "TRY will add to the stack",
        std::string At;
        bool Errored = false;
        try {
            At = AT; TRY("oh no", throw error("error!", "other line"));
        } catch (const error& Error) {
            Errored = true;
            EXPECT_EQUAL(Error.Message, "oh no: error!");
            EXPECT_EQUAL(Error.At, "other line\n    " + At);
        }
        EXPECT_EQUAL(Errored, true);
    );

    TEST
    (   "expect equal throws a consistent message",
        const int A = 3;
        EXPECT_THROW(EXPECT_EQUAL(A, 1), "expected A to equal 1");
    );

    TEST
    (   "expect equal throws a consistent message",
        const int A = 3;
        const int B = 5;
        EXPECT_THROW(EXPECT_EQUAL(A, B), "expected A to equal B");
    );

    TEST
    (   "expect equal throws a consistent message",
        const int C = 0;
        EXPECT_THROW(EXPECT_EQUAL(-1, C), "expected -1 to equal C");
    );

    TEST
    (   "expecting the wrong error will throw",
        EXPECT_THROW
        (   EXPECT_THROW(throw error("oh no!", AT), "uh oh."),
            "expected `throw error(\"oh no!\", AT)` to throw \"uh oh.\" but it threw \"oh no!\""
        );
    );

    TEST
    (   "expecting a throw when there wasn't will throw:",
        EXPECT_THROW
        (   EXPECT_THROW(((void)0), "could throw"),
            "expected `((void)0)` to throw \"could throw\""
        );
    );

    TEST("assert works", ASSERT(100 == 100));

    TEST("throw works", EXPECT_THROW(ASSERT(1 == 2), "expected 1 == 2"));

    TEST
    (   "capturer works with cout",
        capturer Capturer(std::cout);
        std::cout << "hey you!";
        std::cout << " are you\n there?";
        EXPECT_EQUAL(Capturer.pull(), "hey you! are you\n there?");
    );

    TEST
    (   "Calling capturer::pull() clears out buffer:",
        capturer Capturer(std::cout);
        std::cout << "hey you!";
        EXPECT_EQUAL(Capturer.pull(), "hey you!");
        std::cout << "are you cleared?";
        EXPECT_EQUAL(Capturer.pull(), "are you cleared?");
    );

    TEST
    (   "capturer works with cerr",
        capturer Capturer(std::cerr);
        std::cerr << "there is good;";
        std::cerr << "\n let me know if you think so, too\n";
        EXPECT_EQUAL(Capturer.pull(), "there is good;\n let me know if you think so, too\n");
    );
}
#endif

TMVB
