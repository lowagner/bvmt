#include "optional.h"

#ifndef NDEBUG
#include "string.h"
#endif

BVMT

#ifndef NDEBUG
namespace test
{   struct color 
    {   u8 R, G, B;
    };
};
using test::color;
using test::noisy;
void test__core__optional()
{   TEST
    (   "default construction + reset() can be called on a null state",
        optional<noisy> OptionalNoisy;
        ASSERT(OptionalNoisy == Null);
        EXPECT_THROW(*OptionalNoisy, "optional is Null");

        OptionalNoisy.reset();

        ASSERT(OptionalNoisy == Null);
        EXPECT_THROW(*OptionalNoisy, "optional is Null");
    );

    TEST
    (   "construction and reset() works with a value",
        optional<noisy> OptionalNoisy(noisy(3));
        // Takes some moving into place, since we create the noisy in this thread:
        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(3)noisy(3){MC}~noisy(-3)");
        ASSERT(OptionalNoisy != Null);
        EXPECT_EQUAL(*OptionalNoisy, noisy(3));
        // Ignore output for equality comparison:
        ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));

        OptionalNoisy.reset();

        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(3)");
        ASSERT(OptionalNoisy == Null);
        EXPECT_THROW(*OptionalNoisy, "optional is Null");
    );

    TEST
    (   "takeValue",
        TEST
        (   "takeValue() works if not Null",
            {   optional<noisy> OptionalSource(noisy(3));
                optional<noisy> OptionalSink;
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction

                {   noisy Temp = OptionalSource.takeValue();
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(3){MC}~noisy(-3)");
                    ASSERT(OptionalSource == Null);

                    OptionalSink.value(std::move(Temp));
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(3){MC}");
                    ASSERT(OptionalSink != Null);
                }
                // Temp is dying here:
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(-3)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(3)");
        );

        TEST
        (   "takeValue() throws if Null",
            optional<noisy> Optional;
            EXPECT_THROW(Optional.takeValue(), "optional is Null");
        );
    )

    TEST
    (   "move assignment and move construction work",
        TEST
        (   "for incoming value",
            TEST
            (   "move construction works",
                {   optional<noisy> OptionalSource(noisy(10));
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction

                    {   optional<noisy> OptionalSink(std::move(OptionalSource));
                
                        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(10){MC}~noisy(-10)");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink != Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(10)");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );

            TEST
            (   "move assignment works from null state",
                {   optional<noisy> OptionalSource(noisy(10));
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction

                    {   // need to make sure we don't do a move constructor by accident:
                        optional<noisy> OptionalSink;
                        OptionalSink.reset();
                        ASSERT(OptionalSink == Null);
               
                        OptionalSink = std::move(OptionalSource);

                        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(10){MC}~noisy(-10)");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink != Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(10)");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );

            TEST
            (   "move assignment works from existing value",
                {   optional<noisy> OptionalSource(noisy(10));
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction

                    {   // need to make sure we don't do a move constructor by accident:
                        optional<noisy> OptionalSink(noisy(300));
                        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(300)noisy(300){MC}~noisy(-300)");
                        ASSERT(OptionalSink != Null);
               
                        OptionalSink = std::move(OptionalSource);

                        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(300)noisy(10){MC}~noisy(-10)");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink != Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(10)");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );
        );

        TEST
        (   "for incoming null",
            TEST
            (   "move construction works",
                {   optional<noisy> OptionalSource;

                    {   optional<noisy> OptionalSink(std::move(OptionalSource));
                
                        EXPECT_EQUAL(TestPrintOutput.pull(), "");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink == Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );

            TEST
            (   "move assignment works from null state",
                {   optional<noisy> OptionalSource;

                    {   // need to make sure we don't do a move constructor by accident:
                        optional<noisy> OptionalSink;
                        OptionalSink.reset();
                        ASSERT(OptionalSink == Null);
               
                        OptionalSink = std::move(OptionalSource);

                        EXPECT_EQUAL(TestPrintOutput.pull(), "");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink == Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );

            TEST
            (   "move assignment works from existing value",
                {   optional<noisy> OptionalSource;

                    {   // need to make sure we don't do a move constructor by accident:
                        optional<noisy> OptionalSink(noisy(300));
                        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(300)noisy(300){MC}~noisy(-300)");
                        ASSERT(OptionalSink != Null);
               
                        OptionalSink = std::move(OptionalSource);

                        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(300)");
                        ASSERT(OptionalSource == Null);
                        ASSERT(OptionalSink == Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            );
        );

        TEST
        (   "a moved null",
            TEST
            (   "moved null can be converted back to a value",
                {   optional<noisy> Moved;
                    ASSERT(Moved == Null);

                    optional<noisy> Unmoved(std::move(Moved));
                    ASSERT(Moved == Null);

                    Moved.value(noisy(5));
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(5)noisy(5){MC}~noisy(-5)");
                    ASSERT(Moved != Null);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(5)");
            );
        );

        TEST
        (   "a moved value",
            TEST
            (   "moved value can be converted back to a value",
                {   optional<noisy> Moved(noisy(7));
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(7)noisy(7){MC}~noisy(-7)");
                    ASSERT(Moved != Null);

                    {   optional<noisy> Unmoved(std::move(Moved));
                        ASSERT(Moved == Null);
                    }
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(7){MC}~noisy(-7)~noisy(7)");

                    Moved.value(noisy(5));
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(5)noisy(5){MC}~noisy(-5)");
                    ASSERT(Moved != Null);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(5)");
            );
        );
    );

    TEST
    (   "construction and reset() works with a pointer",
        TEST
        (   "default constructor",
            optional<noisy &> OptionalNoisy;
            ASSERT(OptionalNoisy == Null)
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );

        TEST
        (   "reset works on reference type",
            noisy Source(120);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(120)");
            {   optional<noisy &> OptionalNoisy(Source);

                ASSERT(OptionalNoisy != Null)
                EXPECT_EQUAL(OptionalNoisy->Value, 120);
                OptionalNoisy->Value = 4;

                EXPECT_EQUAL(Source.Value, 4);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );
    );

    /* TODO:
    TEST
    (   "construction works for non-reference type",
        TEST
        (   "empty optional copy constructor",
            optional<noisy> Source;
            optional<noisy> Destination(Source);
            ASSERT(Destination == Null)
        );

        TEST
        (   "non-empty optional copy constructor",
            optional<noisy> Source(noisy(100));
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100)");

            {   optional<noisy> Destination(Source);

                ASSERT(Destination != Null);
                EXPECT_EQUAL(Destination->Value, 100);
                Destination->Value = 1234;
                EXPECT_EQUAL(Source->Value, 100);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(1234)");
        );
    );
    */

    TEST
    (   "doesn't take up too much space",
        EXPECT_EQUAL(sizeof(optional<color>), 4 * sizeof(u8));
        optional<color> ColorArray[2];
        EXPECT_EQUAL(size_t((u8 *)(&ColorArray[1]) - (u8 *)(&ColorArray[0])), 4 * sizeof(u8));
    );
}
#endif

TMVB
