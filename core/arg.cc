#include "arg.h"

#ifndef NDEBUG
#include "error.h"
#include "string.h"
#endif

BVMT
#ifndef NDEBUG
using test::noisy;
void test__core__arg()
{   TEST
    (   "arg::in",
        TEST
        (   "works with const reference",
            string Wow("hello world");

            auto ArgIn = arg::in(Wow);

            EXPECT_EQUAL(*ArgIn, "hello world");
            EXPECT_EQUAL(ArgIn->contains("ello worl"), True);
        );

        TEST
        (   "works with noisy const reference",
            noisy Wow(100);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100)");

            {   auto ArgIn = arg::in(Wow);
                EXPECT_EQUAL(ArgIn->Value, 100);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), ""); // doesn't descope at arg::~in
        );

        /* TODO: static compile-time tests:
        TEST
        (   // should fail at compile-time since it's a temporary:
            auto ArgIn = arg::in(string("hi"));
        );
        */
    );

    TEST
    (   "arg::out",
        TEST
        (   "works with reference",
            string Wow("hello world");
            auto ArgOut = arg::out(Wow);

            string &Result = (ArgOut = "only can output, not check this");

            EXPECT_EQUAL(Wow, "only can output, not check this");
            // technically you can check the output after you set it, i.e., via
            // looking at the result of Result here, but you can't see what it was
            // before you set it in the first place.
            EXPECT_EQUAL(&Result, &Wow);
        );

        TEST
        (   "works with noisy reference",
            noisy Wow(100);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100)");

            {   auto ArgOut = arg::out(Wow);
                ArgOut = noisy(1000);
                EXPECT_EQUAL
                (   TestPrintOutput.pull(),
                    "noisy(1000)noisy(1000){MA}~noisy(-1000)"
                );
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), ""); // doesn't descope at arg::~out
        );

        /* TODO: static compile-time tests:
        TEST
        (   // should fail at compile-time since it's a temporary:
            auto ArgOut = arg::out(string("hi"));
        );
        */
    );

    TEST
    (   "arg::resettableOut",
        TEST
        (   "works with reference, by default resets things",
            noisy Wow(123);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(123)");

            {   auto ArgResettableOut = arg::resettableOut(Wow);
                // moves original value over into resettableOut:
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(123){MC}");
            }

            // moves back the original value, destructs the held value:
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(123){MA}~noisy(-123)");
            EXPECT_EQUAL(Wow, noisy(123));
        );

        TEST
        (   "works with reference, does not reset if set equal to something",
            string Wow("hello world");

            {   auto ArgResettableOut = arg::resettableOut(Wow);
                ArgResettableOut = "hi earth";
            }

            EXPECT_EQUAL(Wow, "hi earth");
        );

        TEST
        (   "works with reference, does not reset if set equal to moved something",
            noisy Wow(123);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(123)");

            {   auto ArgResettableOut = arg::resettableOut(Wow);
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(123){MC}");

                ArgResettableOut = noisy(456);

                EXPECT_EQUAL
                (   TestPrintOutput.pull(),
                    "noisy(456)noisy(456){MA}~noisy(-456)"
                );
            }

            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(123)");
            EXPECT_EQUAL(Wow, noisy(456));
        );

        /* TODO: static compile-time tests:
        TEST
        (   // should fail at compile-time since it's a temporary:
            auto ArgOut = arg::out(string("hi"));
        );
        */
    );

    TEST
    (   "arg::io",
        TEST
        (   "works with reference",
            string Wow("hello world");
            auto ArgIo = arg::io(Wow);

            ArgIo = "can check this";

            EXPECT_EQUAL(*ArgIo, "can check this");
            EXPECT_EQUAL(ArgIo->contains("that"), False);
            EXPECT_EQUAL(Wow, "can check this");
        );

        TEST
        (   "works with noisy reference",
            noisy Wow(100);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100)");

            {   auto ArgIo = arg::io(Wow);

                noisy &Result = (ArgIo = noisy(1000));
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(1000)noisy(1000){MA}~noisy(-1000)");

                EXPECT_EQUAL(Wow.Value, 1000);
                EXPECT_EQUAL((*ArgIo).Value, 1000);
                EXPECT_EQUAL(ArgIo->Value, 1000);
                EXPECT_EQUAL(Result.Value, 1000);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), ""); // doesn't descope at arg::~out
        );

        /* TODO: static compile-time tests:
        TEST
        (   // should fail at compile-time since it's a temporary:
            auto ArgIo = arg::out(string("hi"));
        );
        */
    );
}
#endif

TMVB
