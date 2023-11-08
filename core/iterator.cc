#include "iterator.h"

#ifndef NDEBUG
#include "array.h"
#include "string.h"
#endif

BVMT

#ifndef NDEBUG
using test::noisy;

class noisyKernel : public iteratorKernel<noisy &>
{   noisy Noisy;
    noisyKernel(int Count)
    :   iteratorKernel<noisy &>
        ({  .next = [](iteratorKernel<noisy &> *BaseKernelSelf)
            {   CAST_DEFINE(noisyKernel *, Self, BaseKernelSelf);
                if (Self->Noisy.Value > 0)
                {   --(Self->Noisy.Value);
                    return optional<noisy &>(Self->Noisy);
                }
                return optional<noisy &>();
            },
        }),
        Noisy(Count)
    {}
    ~noisyKernel()
    {   std::cout << "~noisyKernel()";
    }
public:
    static iterator<noisy &> toIterator(int Count)
    {   return iterator<noisy &>
        (   pointer<iteratorKernel<noisy &>>
            (   new noisyKernel(Count),
                [](iteratorKernel<noisy &> *Kernel)
                {   // need to cast to the real type for the destructor to work properly:
                    delete (noisyKernel *)Kernel;
                }
            )
        );
    }
};

iterator<noisy &> test::noisyIterator(int Count)
{   return noisyKernel::toIterator(Count);
}

void test__core__iterator()
{   TEST
    (   "iterator works with custom functions (and does cleanup)",
        noisy Noisy0(0);
        noisy Noisy1(1);
        noisy Noisy2(2);
        noisy Noisy3(3);
        ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore constructions here
        {   iterator<noisy &> NoisyIterator = test::noisyIterator(4);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(4)");
            EXPECT_POINTER_EQUAL(NoisyIterator.next(), Noisy3);
            EXPECT_POINTER_EQUAL(NoisyIterator.next(), Noisy2);
            EXPECT_POINTER_EQUAL(NoisyIterator.next(), Noisy1);
            EXPECT_POINTER_EQUAL(NoisyIterator.next(), Noisy0);
            EXPECT_EQUAL(NoisyIterator.next(), Null);
            EXPECT_EQUAL(TestPrintOutput.pull(), ""); // no constructions/destructions here
        }
        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisyKernel()~noisy(0)");
    );

    TEST
    (   "iterator works with standard for-loop",
        {   iterator<noisy &> NoisyIterator = test::noisyIterator(10);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(10)");
            array<int> Array;
            for (noisy &Noisy : NoisyIterator)
            {   Array.append(Noisy.Value);
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), ""); // no constructions/destructions here
            EXPECT_EQUAL(Array, array<int>({9, 8, 7, 6, 5, 4, 3, 2, 1, 0}));
        }
        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisyKernel()~noisy(0)");
    );
    
    TEST
    (   "iterator::range works",
        TEST
        (   "iterator::range works as expected for an int",
            auto Range = iterator<int>::range(100);
            int I = 0;
            for (int R : Range)
            {   EXPECT_EQUAL(R, I++);
            }
            EXPECT_EQUAL(I, 100);
        );

        TEST
        (   "iterator::range works as expected for a double",
            auto Range = iterator<dbl>::range(100.0);
            dbl D = 0.0;
            for (dbl R : Range)
            {   EXPECT_EQUAL(R, D++);
            }
            EXPECT_EQUAL(D, 100.0);
        );
    );

    TEST
    (   "checkAny works",
        TEST
        (   "can avoid finding things",
            EXPECT_EQUAL
            (   test::noisyIterator(10).checkAny
                (   [](const noisy &Noisy)
                    {   return Noisy.Value > 10;
                    }
                ),
                False
            );
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(10)~noisyKernel()~noisy(0)");
        );

        TEST
        (   "can find things",
            EXPECT_EQUAL
            (   test::noisyIterator(7).checkAny
                (   [](const noisy &Noisy)
                    {   return Noisy.Value == 3;
                    }
                ),
                True
            );
            // Note we created a noisy in the iterator and stopped early (before 0):
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(7)~noisyKernel()~noisy(3)");
        );
    );

    TEST
    (   "iterate works",
        TEST
        (   "can map to a different type (noisy -> int)",
            EXPECT_EQUAL
            (   array<int>
                (   test::noisyIterator(10).$ iterate<int>
                    (   [](noisy &Noisy)
                        {   return optional<int>(Noisy.Value);
                        }
                    )
                ),
                array<int>({9, 8, 7, 6, 5, 4, 3, 2, 1, 0})
            );
        );

        TEST
        (   "can map with a filter",
            EXPECT_EQUAL
            (   array<int>
                (   iterator<int>::range(100).$ iterate<int>
                    (   [](int &Int)
                        {   return Int % 30 == 0 ? optional<int>(-Int) : optional<int>();
                        }
                    )
                ),
                array<int>({0, -30, -60, -90})
            );
        );

        TEST
        (   "scoping/descoping works as desired",
            TEST
            (   "when parent outlives child",
                auto ParentIterator = iterator<int>::range(16);
                {   auto ChildIterator = ParentIterator.$ iterate<int>
                    (   [](int &Int)
                        {   return Int % 3 == 0 ? optional<int>(Int) : optional<int>();
                        }
                    );
                    EXPECT_POINTER_EQUAL(ChildIterator.next(), 0);
                    EXPECT_POINTER_EQUAL(ChildIterator.next(), 3);
                    EXPECT_POINTER_EQUAL(ParentIterator.next(), 4);
                    EXPECT_POINTER_EQUAL(ChildIterator.next(), 6);
                    EXPECT_POINTER_EQUAL(ParentIterator.next(), 7);
                    EXPECT_POINTER_EQUAL(ParentIterator.next(), 8);
                    EXPECT_POINTER_EQUAL(ChildIterator.next(), 9);
                }
                EXPECT_EQUAL
                (   array<int>(std::move(ParentIterator)),
                    array<int>({10, 11, 12, 13, 14, 15})
                );
            );

            TEST
            (   "when child outlives parent",
                auto ParentIterator = iterator<int>::range(16);
                auto ChildIterator = std::move(ParentIterator).$ iterate<int>
                (   [](int &Int)
                    {   return Int % 3 == 0 ? optional<int>(Int) : optional<int>();
                    }
                );
                EXPECT_POINTER_EQUAL(ChildIterator.next(), 0);
                EXPECT_POINTER_EQUAL(ChildIterator.next(), 3);
                EXPECT_POINTER_EQUAL(ParentIterator.next(), 4);
                EXPECT_POINTER_EQUAL(ChildIterator.next(), 6);
                // This is a bit evil, since ParentIterator should be considered "used up"
                // by the std::move, but we allow it:
                EXPECT_POINTER_EQUAL(ParentIterator.next(), 7);
                EXPECT_POINTER_EQUAL(ParentIterator.next(), 8);
                EXPECT_POINTER_EQUAL(ChildIterator.next(), 9);

                // This seems pretty shady but it mimics what happens when a parent is
                // created inside a function and the child is returned.
                // Essentially `iterator` needs to be twice-destructible.
                ParentIterator.~iterator();

                EXPECT_EQUAL
                (   array<int>(std::move(ChildIterator)),
                    array<int>({12, 15})
                );
            );
        );

        // TODO: add tests changing types using optional, e.g., int to string
    );

    TEST
    (   "testing new/delete on a void* for some reason",
        void *Void = new noisy(3);
        EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(3)");
        delete (noisy *)Void; // need a cast here for destructor to work.
        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(3)");
    );
}
#endif

TMVB

