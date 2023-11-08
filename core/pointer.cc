#include "pointer.h"

BVMT

#ifndef NDEBUG

using test::parent;
using test::child;
using test::aunt;

void test__core__pointer()
{   const parent DefaultParent;

    TEST
    (   "pointer",
        TEST
        (   "default pointer is ok with construction/destruction.",
            {   pointer<parent> Pointer;
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );

        TEST
        (   "pointer will only try to \"free\" via the callback once even if it throws.",
            {   pointer<parent> Pointer
                (   new parent(100),
                    [](parent *P)
                    {   std::cout << "descoped";
                        delete P;
                        throw error("OH NO", AT);
                    }
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(100)");

                EXPECT_THROW(Pointer.reset(), "OH NO");

                EXPECT_EQUAL(TestPrintOutput.pull(), "descoped~parent(100)");
                ASSERT(Pointer == Null);

                TRY("should not throw", Pointer.reset());
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );

        TEST
        (   "pointer can destroy",
            {   auto Pointer = pointer<parent>::deleteOnDescope
                (   new parent(3)
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(3)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~parent(3)");
        );

        TEST
        (   "pointer can free child classes correctly",
            {   pointer<parent> Pointer = pointer<parent>::deleteOnDescope
                (   new child("Kid", 3)
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(3)child(Kid, 3)");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~child(Kid, 3)~parent(3)");
        );

        TEST
        (   "pointer can move construct",
            {   pointer<parent> PointerSource
                (   new parent(9),
                    [](parent *P) { delete P; }
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(9)");

                pointer<parent> Pointer = std::move(PointerSource);

                EXPECT_EQUAL(TestPrintOutput.pull(), "");

                // We should clean up after ourselves when moving:
                ASSERT(PointerSource == Null);
                ASSERT(Pointer != Null);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~parent(9)");
        );

        TEST
        (   "pointer can move-assign",
            {   pointer<parent> PointerSource
                (   new parent(5),
                    [](parent *P) { std::cout << "NEW"; delete P; }
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(5)");

                pointer<parent> Pointer
                (   new parent(100),
                    [](parent *P) { std::cout << "OLD"; delete P; }
                );

                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(100)");

                Pointer = std::move(PointerSource);

                EXPECT_EQUAL(TestPrintOutput.pull(), "OLD~parent(100)");

                // We should clean up after ourselves when moving:
                ASSERT(PointerSource == Null);
                ASSERT(Pointer != Null);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "NEW~parent(5)");
        );

        TEST
        (   "pointer can abdicate",
            {   pointer<parent> PointerDestination;
                {   pointer<parent> PointerSource
                    (   new parent(9),
                        [](parent *P) { std::cout << "DESTRUCTION"; delete P; }
                    );
                    EXPECT_EQUAL(TestPrintOutput.pull(), "parent(9)");

                    PointerDestination = PointerSource.abdicate();

                    EXPECT_EQUAL(TestPrintOutput.pull(), "");

                    // Source should keep the pointer:
                    ASSERT(&*PointerSource == &*PointerDestination);
                    ASSERT(PointerSource != Null);

                    EXPECT_EQUAL(PointerDestination->Value, 9);
                    ++(PointerDestination->Value);
                    EXPECT_EQUAL(PointerSource->Value, 10);
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "DESTRUCTION~parent(10)");
        );

        TEST
        (   "safe to destroy pointer twice",
            {   pointer<parent> DestructibleTwice
                (   new parent(1),
                    [](parent *P) { delete P; }
                );
                EXPECT_EQUAL(TestPrintOutput.pull(), "parent(1)");

                DestructibleTwice.~pointer();

                EXPECT_EQUAL(TestPrintOutput.pull(), "~parent(1)");
                ASSERT(DestructibleTwice == Null);
                EXPECT_THROW(DestructibleTwice->Value, "Cannot access fields of null pointer.");
                EXPECT_THROW(*DestructibleTwice, "Cannot access fields of null pointer.");
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );
    );
}
#endif

TMVB
