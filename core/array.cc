#include "array.h"

#ifndef NDEBUG
#include "string.h"
#endif

BVMT

const char *const ArrayTooNegativeIndexErrorMsg = "array indexing was too negative";
const char *const ArrayPoppingErrorMsg = "array cannot pop due to insufficient count";
const char *const ArrayInvalidArgumentErrorMsg = "array was passed an invalid argument";
const char *const ArrayFixedCountErrorMsg = "array is fixed count; cannot change its count";
const char *const ArrayViewEmptyMsg = "arrayView is empty";

#ifndef NDEBUG
using test::noisy;
void test__core__array()
{   TEST
    (   "can create array from an iterator implicitly via move",
        // both by move construction and assignment as well as +=
        array<noisy> Array(std::move(test::noisyIterator(5)));
        int I = 5;
        for (noisy &A : Array.values())
        {   EXPECT_EQUAL(A.Value, --I);
        }
        EXPECT_EQUAL(Array.count(), 5);
        EXPECT_EQUAL(I, 0);

        // keep going, since we want to ensure that operator = overwrites the current values.

        Array.reserve(15);
        ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction/destruction here
        {   auto NoisyIterator = test::noisyIterator(10);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(10)");
            Array = std::move(NoisyIterator);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                // Delete existing elements in the array:
                "~noisy(4)~noisy(3)~noisy(2)~noisy(1)~noisy(0)"
                // Copy-construct the remainder:
                "noisy(9){CC}"
                "noisy(8){CC}"
                "noisy(7){CC}"
                "noisy(6){CC}"
                "noisy(5){CC}"
                "noisy(4){CC}"
                "noisy(3){CC}"
                "noisy(2){CC}"
                "noisy(1){CC}"
                "noisy(0){CC}"
            );
        }
        // make sure noisyIterator cleans up after itself:
        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisyKernel()~noisy(0)");

        I = 10;
        for (noisy &A : Array.values())
        {   EXPECT_EQUAL(A.Value, --I);
        }
        EXPECT_EQUAL(Array.count(), 10);
        EXPECT_EQUAL(I, 0);

        EXPECT_EQUAL(TestPrintOutput.pull(), "");
        // finally the += operation
        {   auto NoisyIterator = test::noisyIterator(5);
            EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(5)");
            Array += std::move(NoisyIterator);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                // Copy-construct the remainder:
                "noisy(4){CC}"
                "noisy(3){CC}"
                "noisy(2){CC}"
                "noisy(1){CC}"
                "noisy(0){CC}"
            );
        }
        // noisyIterator cleanup:
        EXPECT_EQUAL(TestPrintOutput.pull(), "~noisyKernel()~noisy(0)");

        I = 10;
        for (noisy &A : Array.values())
        {   EXPECT_EQUAL(A.Value, --I);
            if (I == 0) I = 5;
        }
        EXPECT_EQUAL(Array.count(), 15);
    );

    TEST
    (   "can create array from a const iterator implicitly with r-value",
        // both by move construction and assignment.
        // testing an implicit r-value here rather than an explicit move:
        array<int> Array(iterator<int>::range(30));
        int I = 0;
        for (int A : Array.values())
        {   EXPECT_EQUAL(A, I++);
        }
        EXPECT_EQUAL(Array.count(), 30);
        EXPECT_EQUAL(I, 30);

        // keep going, since we want to ensure that operator = overwrites the current values.

        // create a new situation, from 1 to 20
        auto Iterator = iterator<int>::range(21);
        Iterator.next();

        Array = std::move(Iterator);
        I = 1;
        for (int A : Array.values())
        {   EXPECT_EQUAL(A, I++);
        }
        EXPECT_EQUAL(Array.count(), 20);
        EXPECT_EQUAL(I, 21);

        // also test the += operation:
        Array += iterator<int>::range(4);
        I = 1;
        for (int A : Array.values())
        {   EXPECT_EQUAL(A, I++);
            if (I == 21) I = 0;
        }
        EXPECT_EQUAL(Array.count(), 24);
    );

    TEST
    (   "can + and += correctly on another array",
        TEST
        (   "can += correctly",
            array<int> Array({1, 2, 3, 4, 5});
            Array += array<int>({10, 11, 12});
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 4, 5, 10, 11, 12}));
        );

        TEST
        (   "can + correctly on another array",
            array<int> Array;
            EXPECT_EQUAL(Array.count(), 0);
            Array = array<int>({1, 2, 3, 4, 5}) + array<dbl>({9.3, 10.1, 11.9});
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 4, 5, 9, 10, 11}));
        );
    );

    TEST
    (   "index access via get()",
        TEST
        (   "works for getting elements inside the array",
            array<int> Array({5, 10, 15});

            EXPECT_POINTER_EQUAL(Array.get(0), 5);
            EXPECT_EQUAL(Array.count(), 3);
            EXPECT_POINTER_EQUAL(Array.get(1), 10);
            EXPECT_EQUAL(Array.count(), 3);
            EXPECT_POINTER_EQUAL(Array.get(2), 15);
            EXPECT_EQUAL(Array.count(), 3);
        );

        TEST
        (   "does NOT resize Array when getting an index above current size",
            array<int> Array;
            EXPECT_EQUAL(Array.count(), 0);

            EXPECT_EQUAL(Array.get(0), Null);

            EXPECT_EQUAL(Array.count(), 0);

            EXPECT_EQUAL(Array.get(5), Null);

            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "complains about too negative indexing when getting",
            array<int> Array;

            EXPECT_THROW(*Array.get(-1), ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(*Array.get(-2), ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(*Array.get(-3), ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(*Array.get(-4), ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(*Array.get(-5), ArrayTooNegativeIndexErrorMsg);

            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "negative indexing works as expected",
            array<int> Array({0, 1, 2, 3});

            EXPECT_POINTER_EQUAL(Array.get(-1), 3);
            EXPECT_POINTER_EQUAL(Array.get(-2), 2);
            EXPECT_POINTER_EQUAL(Array.get(-3), 1);
            EXPECT_POINTER_EQUAL(Array.get(-4), 0);

            ASSERT(&*Array.get(-3) == &Array[1]);
        );
    );

    TEST
    (   "index access via operator[]",
        TEST
        (   "resizes Array when setting an index above current count",
            array<int> Array;
            EXPECT_EQUAL(Array.count(), 0);

            Array[3] = 5;

            EXPECT_EQUAL(Array.count(), 4);
            EXPECT_EQUAL(Array[0], 0);
            EXPECT_EQUAL(Array[1], 0);
            EXPECT_EQUAL(Array[2], 0);
            EXPECT_EQUAL(Array[3], 5);
        );

        TEST
        (   "resizes Array when setting an index JUST above current count",
            // plus checking in on memory management (new/delete).
            {   array<noisy> Array;
                Array.reserve(2);
                EXPECT_EQUAL(TestPrintOutput.pull(), "");

                Array[0] = noisy(100);
                EXPECT_EQUAL(TestPrintOutput.pull(),
                    // The new value we're assigning:
                    "noisy(100)"
                    // Default that gets created when calling Array[0] by itself:
                    "noisy(-1)"
                    // The move assignment from noisy(100) to Array[0]:
                    "noisy(100){MA}"
                    // Deleting the original value we created with noisy(100)
                    // The internal memory should have been moved from this to Array[0]
                    // So this should not be doing any memory deallocation specifically.
                    "~noisy(-100)"
                );
                EXPECT_EQUAL(Array.count(), 1);

                Array[1] = noisy(200);
                EXPECT_EQUAL(TestPrintOutput.pull(),
                    // The new value we're assigning:
                    "noisy(200)"
                    // Default that gets created when calling Array[1] by itself:
                    "noisy(-1)"
                    // The move assignment from noisy(200) to Array[1]:
                    "noisy(200){MA}"
                    // Deleting the original value we created with noisy(200)
                    // The internal memory should have been moved from this to Array[0]
                    // So this should not be doing any memory deallocation specifically.
                    "~noisy(-200)"
                );
                //noisy(200)noisy(100){CC}~noisy(100)noisy(-1)noisy{MA 200}~noisy(-200)
                EXPECT_EQUAL(Array.count(), 2);

                {
                    const array<noisy> &ConstArray = Array;
                    EXPECT_EQUAL(ConstArray[0], noisy(100));
                    EXPECT_EQUAL(ConstArray[1], noisy(200));
                    // When constant, the array is effectively fixed count.
                    // We could change the error message but it's probably fine.
                    EXPECT_THROW(ConstArray[2], ArrayFixedCountErrorMsg);
                }
                // Ignore output from creating and deleting noisys for comparison:
                ASSERT_STRING(TestPrintOutput.pull(), contains("~noisy"));
            }
            // The Array should free memory:
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(100)~noisy(200)");
        );

        TEST
        (   "resizes Array when setting an index far above current size",
            // also memory management
            {   array<noisy> Array;
                Array.reserve(5);
                EXPECT_EQUAL(Array.count(), 0);
                EXPECT_EQUAL(TestPrintOutput.pull(), "");

                Array[4] = noisy(100);
                EXPECT_EQUAL(TestPrintOutput.pull(),
                    // The new value we're assigning:
                    "noisy(100)"
                    // Creating defaults from index 0 up to 4
                    "noisy(-1)"
                    "noisy(-1)"
                    "noisy(-1)"
                    "noisy(-1)"
                    "noisy(-1)"
                    // Deleting the original value we created with noisy(100)
                    // The internal memory should have been moved from this to Array[0]
                    // So this should not be doing any memory deallocation specifically.
                    "noisy(100){MA}~noisy(-100)"
                );
                EXPECT_EQUAL(Array.count(), 5);

                {   const array<noisy> &ConstArray = Array;
                    EXPECT_EQUAL(ConstArray[0], noisy(-1));
                    EXPECT_EQUAL(ConstArray[1], noisy(-1));
                    EXPECT_EQUAL(ConstArray[2], noisy(-1));
                    EXPECT_EQUAL(ConstArray[3], noisy(-1));
                    EXPECT_EQUAL(ConstArray[4], noisy(100));
                }
                // Ignore output from creating and deleting noisys for comparison:
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));
            }
            // The Array should free memory:
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "~noisy(-1)~noisy(-1)~noisy(-1)~noisy(-1)~noisy(100)"
            );
        );

        TEST
        (   "does not resize const array when getting an index above current size",
            const array<int> Array;
            EXPECT_EQUAL(Array.count(), 0);

            // When constant, the array is effectively fixed size.
            // We could change the error message but it's probably fine:
            EXPECT_THROW(Array[0], ArrayFixedCountErrorMsg);
            EXPECT_THROW(Array[300], ArrayFixedCountErrorMsg);

            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "complains about too negative indexing when getting",
            const array<int> Array;

            EXPECT_THROW(Array[-1], ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(Array[-2], ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(Array[-3], ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(Array[-4], ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(Array[-5], ArrayTooNegativeIndexErrorMsg);

            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "setting the -1'th element just hits the end of the Array",
            array<int> Array;
            Array[0] = 1200;
            EXPECT_EQUAL(Array[-1], 1200);
            EXPECT_EQUAL(Array[0], 1200);
            EXPECT_EQUAL(Array.count(), 1);

            Array[-1] = 12;

            EXPECT_EQUAL(Array[-1], 12);
            EXPECT_EQUAL(Array[0], 12);
            EXPECT_EQUAL(Array.count(), 1);
        );

        TEST
        (   "negative indexing works as expected",
            array<int> Array;
            Array[0] = 0;
            Array[1] = 1;
            Array[2] = 2;
            Array[3] = 3;

            EXPECT_EQUAL(Array[-1], 3);
            EXPECT_EQUAL(Array[-2], 2);
            EXPECT_EQUAL(Array[-3], 1);
            EXPECT_EQUAL(Array[-4], 0);

            Array[-3] = 100;

            EXPECT_EQUAL(Array[0], 0);
            EXPECT_EQUAL(Array[1], 100);
            EXPECT_EQUAL(Array[2], 2);
            EXPECT_EQUAL(Array[3], 3);
            EXPECT_EQUAL(Array[-1], 3);
            EXPECT_EQUAL(Array[-2], 2);
            EXPECT_EQUAL(Array[-3], 100);
            EXPECT_EQUAL(Array[-4], 0);
        );

        TEST
        (   "does complain about setting too negative values",
            array<int> Array;

            Array[0] = 0;
            Array[1] = 1;
            Array[2] = 2;
            Array[3] = 3;

            EXPECT_THROW(Array[-5] = 2, ArrayTooNegativeIndexErrorMsg);
            EXPECT_THROW(Array[-6] = 3, ArrayTooNegativeIndexErrorMsg);
        );
    );

    TEST
    (   "array swap",
        TEST
        (   "using std::swap doesn't recreate internal objects",
            array<noisy> Array1({noisy(3), noisy(40), noisy(500), noisy(1000)});
            array<noisy> Array2({noisy(5), noisy(6), noisy(7)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            std::swap(Array1, Array2);
            // No re-construction or moving at all of noisy instances:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array1, array<noisy>({noisy(5), noisy(6), noisy(7)}));
            EXPECT_EQUAL(Array2, array<noisy>({noisy(3), noisy(40), noisy(500), noisy(1000)}));
        );

        TEST
        (   "can swap out within current size of array",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            noisy Result = Array.swap(1, noisy(40000));
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                // Creating the noisy to be moved:
                "noisy(40000)"
                // Moving out the current occupant:
                "noisy(40){MC}"
                // Moving in:
                "noisy(40000){MA}"
                // Deleting the moved noisy in the array:
                "~noisy(-40000)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(3), noisy(40000), noisy(500)
                })
            );
        );

        TEST
        (   "can swap out above current count of array",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            Array.reserve(6);
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction/moving

            noisy Result = Array.swap(5, noisy(12345));
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                // creating the new element:
                "noisy(12345)"
                // Creating all defaults up to the requested index:
                "noisy(-1)"
                "noisy(-1)"
                "noisy(-1)"
                // Moving out the newly created 5th index element:
                "noisy(-1){MC}"
                // moving in the new element:
                "noisy(12345){MA}"
                // deleting the temporary moved element:
                "~noisy(-12345)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(3), noisy(40), noisy(500), noisy(), noisy(), noisy(12345)
                })
            );
        );

        TEST
        (   "can swap out negative indices, if they are not too negative",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            Array.swap(-1, noisy(505));
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(505)"
                "noisy(500){MC}"
                "noisy(505){MA}"
                "~noisy(500)"
                "~noisy(-505)"
            );

            Array.swap(-3, noisy(2));
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(2)"
                "noisy(3){MC}"
                "noisy(2){MA}"
                "~noisy(3)"
                "~noisy(-2)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(2), noisy(40), noisy(505)
                })
            );
        );
    );

    TEST
    (   "array swapIndices",
        TEST
        (   "can swap out within current count of array",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            Array.swapIndices(1, 0);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(40){MC}"
                "noisy(3){MA}"
                "noisy(40){MA}"
                // deleting the temporary (moved noisy)
                "~noisy(-40)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(40), noisy(3), noisy(500)
                })
            );
        );

        TEST
        (   "can swap out above current count of array",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            Array.reserve(6);
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction/moving

            Array.swapIndices(5, 2);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                // Creating all defaults up to the requested index:
                "noisy(-1)"
                "noisy(-1)"
                "noisy(-1)"
                // Moving out the newly created 5th index element:
                "noisy(-1){MC}"
                // moving in the new elements:
                "noisy(500){MA}"
                "noisy(-1){MA}"
                // deleting the temporary moved element:
                "~noisy(1)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(3), noisy(40), noisy(), noisy(), noisy(), noisy(500)
                })
            );
        );

        TEST
        (   "order doesn't matter when swapping a negative index and a larger index",
            array<noisy> Array1({noisy(3), noisy(40), noisy(500)});
            Array1.reserve(6);
            array<noisy> Array2({noisy(3), noisy(40), noisy(500)});
            Array2.reserve(6);
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction/moving

            Array1.swapIndices(4, -1);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(-1)noisy(-1)"
                "noisy(-1){MC}"
                "noisy(500){MA}noisy(-1){MA}"
                // Deleting the temporary (moved value)
                "~noisy(1)"
            );

            Array2.swapIndices(-1, 4);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(-1)noisy(-1)"
                "noisy(500){MC}"
                "noisy(-1){MA}noisy(500){MA}"
                // Deleting the temporary (moved value)
                "~noisy(-500)"
            );

            EXPECT_EQUAL
            (   Array1,
                array<noisy>
                ({  noisy(3), noisy(40), noisy(), noisy(), noisy(500),
                })
            );
            EXPECT_EQUAL(Array1, Array2);

            Array1.swapIndices(5, -4);
            Array2.swapIndices(-4, 5);

            EXPECT_EQUAL
            (   Array1,
                array<noisy>
                ({  noisy(3), noisy(), noisy(), noisy(), noisy(500), noisy(40),
                })
            );
            EXPECT_EQUAL(Array1, Array2);
        );

        TEST
        (   "can swap out negative indices, if they are not too negative",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            Array.swapIndices(-1, -3);
            EXPECT_EQUAL
            (   TestPrintOutput.pull(),
                "noisy(500){MC}noisy(3){MA}noisy(500){MA}~noisy(-500)"
            );

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(500), noisy(40), noisy(3),
                })
            );
        );

        TEST
        (   "too negative indices will throw and will not modify the array",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            EXPECT_THROW
            (   Array.swapIndices(-1, -4),
                ArrayTooNegativeIndexErrorMsg
            );

            EXPECT_THROW
            (   Array.swapIndices(-5, -3),
                ArrayTooNegativeIndexErrorMsg
            );

            EXPECT_THROW
            (   Array.swapIndices(7, -100),
                ArrayTooNegativeIndexErrorMsg
            );

            EXPECT_THROW
            (   Array.swapIndices(-4, 88),
                ArrayTooNegativeIndexErrorMsg
            );
            
            EXPECT_EQUAL(Array.count(), 3);
        );

        TEST
        (   "same index (positive+positive, negative and positive, or negative+negative) does nothing",
            array<noisy> Array({noisy(3), noisy(40), noisy(500)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            Array.swapIndices(2, 2);
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            Array.swapIndices(-2, 1);
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            Array.swapIndices(-3, -3);
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL
            (   Array,
                array<noisy>
                ({  noisy(3), noisy(40), noisy(500),
                })
            );
        );
    );

    TEST
    (   "array append",
        TEST
        (   "works for simple type",
            array<int> Array;
            Array.append(3);
            EXPECT_EQUAL(Array.count(), 1);

            Array.append(5);
            EXPECT_EQUAL(Array.count(), 2);

            Array.append(100);
            EXPECT_EQUAL(Array.count(), 3);

            EXPECT_EQUAL(Array[0], 3);
            EXPECT_EQUAL(Array[1], 5);
            EXPECT_EQUAL(Array[2], 100);
        );

        TEST
        (   "works with emplace on more complicated type",
            {   array<noisy> Array;
                Array.reserve(3); // to avoid creating/destroying when reallocating internally
                EXPECT_EQUAL(Array.count(), 0);

                // Normal append, will create and destroy a noisy:
                Array.append(noisy(100));
                EXPECT_EQUAL(Array.count(), 1);
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100)noisy(100){MC}~noisy(-100)");

                // Emplace-style append, only creates a noisy:
                Array.appendInPlace(200);
                EXPECT_EQUAL(Array.count(), 2);
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(200)");

                // Emplace-style append:
                Array.appendInPlace(3000);
                EXPECT_EQUAL(Array.count(), 3);
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(3000)");

                {   EXPECT_EQUAL(Array[0], noisy(100));
                    EXPECT_EQUAL(Array[1], noisy(200));
                    EXPECT_EQUAL(Array[2], noisy(3000));
                }
                // Ignore output from constructing comparison noisys:
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));
            }
            // Memory cleanup works as well:
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(100)~noisy(200)~noisy(3000)");
        );
    );

    TEST
    (   "array popping",
        TEST
        (   "pop() without an argument",
            TEST
            (   "removes last element from Array",
                array<const char *> Array;
                Array[0] = "hello";
                Array[1] = "world!";
                Array[2] = "nice day!";
                EXPECT_EQUAL(Array.count(), 3);

                EXPECT_EQUAL(Array.pop(), "nice day!");
                EXPECT_EQUAL(Array.count(), 2);

                EXPECT_EQUAL(Array.pop(), "world!");
                EXPECT_EQUAL(Array.count(), 1);

                EXPECT_EQUAL(Array.pop(), "hello");
                EXPECT_EQUAL(Array.count(), 0);
            );

            TEST
            (   "complains if nothing is in the Array",
                array<const char *> Array;
                EXPECT_THROW(Array.pop(), ArrayPoppingErrorMsg);

                Array[0] = "hello!";
                EXPECT_EQUAL(Array.pop(), "hello!");
                EXPECT_THROW(Array.pop(), ArrayPoppingErrorMsg);
            );

            TEST
            (   "does not copy the last value; pop moves it",
                {   array<noisy> Array;
                    Array[0] = noisy(100);
                    EXPECT_EQUAL(TestPrintOutput.pull(),
                        // The new value we're assigning:
                        "noisy(100)"
                        // Default that gets created when calling Array[0] by itself:
                        "noisy(-1)"
                        // The move assignment from noisy(100) to Array[0]:
                        "noisy(100){MA}"
                        // Deleting the original value we created with noisy(100)
                        "~noisy(-100)"
                    );

                    noisy element = Array.pop();

                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(100){MC}~noisy(-100)");
                }
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(100)");
            );
        );

        TEST
        (   "pop() with an argument",
            TEST
            (   "removes relevant element from Array",
                array<const char *> Array;
                Array[0] = "hello";
                Array[1] = "world!";
                Array[2] = "nice day!";
                Array[3] = "wow";
                EXPECT_EQUAL(Array.count(), 4);

                EXPECT_EQUAL(Array.pop(0), "hello");
                EXPECT_EQUAL(Array.count(), 3);

                EXPECT_EQUAL(Array.pop(1), "nice day!");
                EXPECT_EQUAL(Array.count(), 2);

                EXPECT_EQUAL(Array.pop(-1), "wow");
                EXPECT_EQUAL(Array.count(), 1);
            );

            TEST
            (   "complains if nothing is in the Array",
                array<const char *> Array;
                EXPECT_THROW(Array.pop(1), ArrayPoppingErrorMsg);
            );

            TEST
            (   "complains if index is too high",
                array<const char *> Array;
                Array[0] = "hello!";
                EXPECT_THROW(Array.pop(10), ArrayPoppingErrorMsg);
                EXPECT_THROW(Array.pop(1), ArrayPoppingErrorMsg);
            );

            TEST
            (   "does not copy the popped value; pop moves it",
                {   array<noisy> Array;
                    Array[0] = noisy(100);
                    Array[1] = noisy(200);
                    Array[2] = noisy(300);
                    ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction

                    noisy element = Array.pop(1);

                    // Move constructs the noisy for the pop (noisy(200)) but
                    // also needs to move the last element down:
                    EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(200){MC}noisy(300){MA}~noisy(-300)");
                }
                // First descopes the popped element, then cleans up the array:
                EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(200)~noisy(100)~noisy(300)");
            );
        );
    );

    TEST
    (   "insert",
        TEST
        (   "insert with negative index doesn't work if Array is empty",
            array<int> Array;
            EXPECT_THROW(Array.insert(-1, 1234), ArrayTooNegativeIndexErrorMsg);
        );

        TEST
        (   "insert with zero times does nothing",
            array<int> Array;
            Array.insert(0, 1234, 0);
            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "insert past the end of the array resizes the array",
            {   // Insert with the default Count (== 1).
                array<int> Array;
                Array.insert(0, 1234);
                EXPECT_EQUAL(Array, array<int>({1234}));
                Array.insert(3, 4);
                EXPECT_EQUAL(Array, array<int>({1234, 0, 0, 4}));
            }

            {   // Insert past Index = 0 when the array is empty, with default Count
                array<int> Array;
                Array.insert(5, 1234);
                EXPECT_EQUAL(Array, array<int>({0, 0, 0, 0, 0, 1234}));
            }

            {   // Insert past Index = 0 when the array is empty, with Count
                array<int> Array;
                Array.insert(5, 1, 3);
                EXPECT_EQUAL(Array, array<int>({0, 0, 0, 0, 0, 1, 1, 1}));
            }

            {   // Insert with a Count
                array<int> Array;
                Array.insert(0, 123, 2);
                EXPECT_EQUAL(Array, array<int>({123, 123}));
                Array.insert(3, 4, 5);
                EXPECT_EQUAL(Array, array<int>({123, 123, 0, 4, 4, 4, 4, 4}));
            }
        );

        TEST
        (   "insert with default times adds one element",
            array<int> Array;
            Array.insert(0, 100);
            EXPECT_EQUAL(Array.count(), 1);
            EXPECT_EQUAL(Array[0], 100);

            Array.insert(0, 200);
            EXPECT_EQUAL(Array.count(), 2);
            EXPECT_EQUAL(Array[0], 200);
            EXPECT_EQUAL(Array[1], 100);

            Array.insert(1, 300);
            EXPECT_EQUAL(Array.count(), 3);
            EXPECT_EQUAL(Array[0], 200);
            EXPECT_EQUAL(Array[1], 300);
            EXPECT_EQUAL(Array[2], 100);
        );

        TEST
        (   "insert with specified times adds that many elements",
            array<int> Array;
            Array.insert(0, 100, 2);
            EXPECT_EQUAL(Array.count(), 2);
            EXPECT_EQUAL(Array[0], 100);
            EXPECT_EQUAL(Array[1], 100);

            Array.insert(1, 200, 1);
            EXPECT_EQUAL(Array.count(), 3);
            EXPECT_EQUAL(Array[0], 100);
            EXPECT_EQUAL(Array[1], 200);
            EXPECT_EQUAL(Array[2], 100);

            Array.insert(2, 300, 5);
            EXPECT_EQUAL(Array.count(), 8);
            EXPECT_EQUAL(Array[0], 100);
            EXPECT_EQUAL(Array[1], 200);
            EXPECT_EQUAL(Array[2], 300);
            EXPECT_EQUAL(Array[3], 300);
            EXPECT_EQUAL(Array[4], 300);
            EXPECT_EQUAL(Array[5], 300);
            EXPECT_EQUAL(Array[6], 300);
            EXPECT_EQUAL(Array[7], 100);
        );
    );

    TEST
    (   "remove",
        TEST
        (   "remove with no valid candidates returns 0 and doesn't modify array",
            array<int> Array({1, 2, 3, 5, 10, 12, 410});
            EXPECT_EQUAL(Array.remove(12345), 0);
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 5, 10, 12, 410}));

            array<noisy> NoisyArray({noisy(1), noisy(2)});
            EXPECT_EQUAL(NoisyArray.remove(noisy(3)), 0);
            EXPECT_EQUAL(NoisyArray, array<noisy>({noisy(1), noisy(2)}));
        );

        TEST
        (   "remove with one valid candidate returns 1 and modifies array",
            array<int> Array({1, 2, 3, 5, 10, 12, 410});
            EXPECT_EQUAL(Array.remove(5), 1);
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 10, 12, 410}));

            array<noisy> NoisyArray({noisy(1), noisy(2)});
            EXPECT_EQUAL(NoisyArray.remove(noisy(1)), 1);
            EXPECT_EQUAL(NoisyArray, array<noisy>({noisy(2)}));
        );

        TEST
        (   "remove with multiple valid candidate returns that count",
            array<int> Array({5, 3, 5, 5, 5, 5, 2, 1, 5});
            EXPECT_EQUAL(Array.remove(5), 6);
            EXPECT_EQUAL(Array, array<int>({3, 2, 1}));

            array<noisy> NoisyArray({noisy(2), noisy(1), noisy(2), noisy(3)});
            EXPECT_EQUAL(NoisyArray.remove(noisy(2)), 2);
            EXPECT_EQUAL(NoisyArray, array<noisy>({noisy(1), noisy(3)}));
        );
    );

    TEST
    (   "erase",
        TEST
        (   "erase with negative index doesn't work if Array is empty",
            array<int> Array;
            EXPECT_THROW(Array.erase(-1, 1234), ArrayTooNegativeIndexErrorMsg);
        );

        TEST
        (   "erase with negative index works if Array has elements",
            array<int> Array({1, 2, 3, 4, 5, 6, 7, 8});
            Array.erase(-1);
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 4, 5, 6, 7}));
            Array.erase(-2);
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 4, 5, 7}));
            Array.erase(-3);
            EXPECT_EQUAL(Array, array<int>({1, 2, 3, 5, 7}));
            Array.erase(-4, 2);
            EXPECT_EQUAL(Array, array<int>({1, 5, 7}));
        );

        TEST
        (   "erase with zero count does nothing",
            array<int> Array;
            Array[0] = 0;
            Array[1] = 1;
            Array[2] = 2;
            Array[3] = 3;

            Array.erase(0, 0);
            Array.erase(1, 0);
            Array.erase(2, 0);
            Array.erase(3, 0);

            EXPECT_EQUAL(Array.count(), 4);
            EXPECT_EQUAL(Array[0], 0);
            EXPECT_EQUAL(Array[1], 1);
            EXPECT_EQUAL(Array[2], 2);
            EXPECT_EQUAL(Array[3], 3);
        );

        TEST
        (   "erase with default count removes one element",
            array<int> Array;
            for (int I = 0; I < 10; ++I)
            {   Array[I] = I;
            }

            Array.erase(0);
            EXPECT_EQUAL(Array.count(), 9);
            EXPECT_EQUAL(Array[0], 1);

            Array.erase(1);
            EXPECT_EQUAL(Array.count(), 8);
            EXPECT_EQUAL(Array[1], 3);

            Array.erase(5);
            EXPECT_EQUAL(Array.count(), 7);
            EXPECT_EQUAL(Array[0], 1);
            EXPECT_EQUAL(Array[1], 3);
            EXPECT_EQUAL(Array[2], 4);
            EXPECT_EQUAL(Array[3], 5);
            EXPECT_EQUAL(Array[4], 6);
            EXPECT_EQUAL(Array[5], 8);
            EXPECT_EQUAL(Array[6], 9);
        );

        TEST
        (   "erase with specified count deletes that many elements",
            array<int> Array;
            for (int I = 0; I < 100; ++I)
            {   Array[I] = I;
            }

            Array.erase(5, 90);
            EXPECT_EQUAL(Array.count(), 10);
            for (int I = 0; I < 5; ++I)
            {   EXPECT_EQUAL(Array[I], I);
                EXPECT_EQUAL(Array[5 + I], 95 + I);
            }

            Array.erase(2, 6);
            EXPECT_EQUAL(Array.count(), 4);
            EXPECT_EQUAL(Array[0], 0);
            EXPECT_EQUAL(Array[1], 1);
            EXPECT_EQUAL(Array[2], 98);
            EXPECT_EQUAL(Array[3], 99);
        );
    );

    TEST
    (   "findFirst",
        TEST
        (   "findFirst with no array elements returns false",
            array<int> Array;
            index Index = -1;
            EXPECT_EQUAL(Array.findFirst(100, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            EXPECT_EQUAL(Array.findFirst(300, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            EXPECT_EQUAL(Array.findFirst(-50, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            // Does not affect the array:
            EXPECT_EQUAL(Array.count(), 0);
        );

        TEST
        (   "findFirst with no match returns false and does not change Index",
            array<int> Array({0, 100, 22, 3000, 44, 5005, 60006});
            EXPECT_EQUAL(Array.count(), 7);
            index Index = -1;
            EXPECT_EQUAL(Array.findFirst(101, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            EXPECT_EQUAL(Array.findFirst(300, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            EXPECT_EQUAL(Array.findFirst(-50, determining(Index)), False);
            EXPECT_EQUAL(Index, -1);
            // Does not affect the array:
            EXPECT_EQUAL(Array, array<int>({0, 100, 22, 3000, 44, 5005, 60006}));
        );

        TEST
        (   "findFirst returns match",
            array<int> Array({-1, 100, 22, 3000, 44, 5005, 60006, 7});
            EXPECT_EQUAL(Array.count(), 8);
            for (index I = 0; I < Array.count(); ++I)
            {   index FoundIndex = -1;
                EXPECT_EQUAL(Array.findFirst(Array[I], determining(FoundIndex)), True);
                EXPECT_EQUAL(FoundIndex, I);
            }
            // Does not affect the array:
            EXPECT_EQUAL(Array.count(), 8);
        );

        TEST
        (   "findFirst returns first match",
            array<int> Array({0, 1, 2, 2, 2, 3, 4, 5, 6, 2, 2, 2, 3, 0, 3, 1});
            index Index = -1;
            EXPECT_EQUAL(Array.findFirst(0, determining(Index)), True);
            EXPECT_EQUAL(Index, 0);
            EXPECT_EQUAL(Array.findFirst(1, determining(Index)), True);
            EXPECT_EQUAL(Index, 1);
            EXPECT_EQUAL(Array.findFirst(2, determining(Index)), True);
            EXPECT_EQUAL(Index, 2);
            EXPECT_EQUAL(Array.findFirst(3, determining(Index)), True);
            EXPECT_EQUAL(Index, 5);
            // Does not affect the array:
            EXPECT_EQUAL(Array, array<int>({0, 1, 2, 2, 2, 3, 4, 5, 6, 2, 2, 2, 3, 0, 3, 1}));
        );
    );

    TEST
    (   "array reversing",
        TEST
        (   "returns pointer to original array",
            array<int> Array({1000,2,3,4,5,6,700});
            EXPECT_EQUAL(&Array.reverse(), &Array);
        );

        TEST
        (   "reverses array",
            array<int> Array({1000,2,3,40,500,6,700});
            EXPECT_EQUAL
            (   Array.reverse(),
                array<int>({700,6,500,40,3,2,1000})
            );
        );
    );

    TEST
    (   "array sorting",
        TEST
        (   "returns pointer to original array",
            array<int> Array({1,2,3,4,5,-5,-4,-3,-2,0,-1});
            EXPECT_EQUAL(&Array.sort(), &Array);
        );

        TEST
        (   "sorts array",
            array<int> Array({1,2,3,4,5,-5,-4,-3,-2,0,-1});
            EXPECT_EQUAL
            (   Array.sort(),
                array<int>({-5,-4,-3,-2,-1,0,1,2,3,4,5,})
            );
        );
    );

    TEST
    (   "array resize via array::count(index)",
        TEST
        (   "works for simple type",
            array<int> Array;
            EXPECT_EQUAL(Array.count(), 0);

            Array.count(5);
            EXPECT_EQUAL(Array.count(), 5);
            EXPECT_EQUAL(Array[0], 0);
            EXPECT_EQUAL(Array[1], 0);
            EXPECT_EQUAL(Array[2], 0);
            EXPECT_EQUAL(Array[3], 0);
            EXPECT_EQUAL(Array[4], 0);
        );

        TEST
        (   "works for more complicated type",
            {   array<noisy> Array;
                Array.count(2);
                EXPECT_EQUAL(TestPrintOutput.pull(), "noisy(-1)noisy(-1)");
                EXPECT_EQUAL(Array.count(), 2);

                {   EXPECT_EQUAL(Array[0], noisy(-1));
                    EXPECT_EQUAL(Array[1], noisy(-1));
                }
                // Ignore output from constructing comparison noisys:
                ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));
            }
            EXPECT_EQUAL(TestPrintOutput.pull(), "~noisy(-1)~noisy(-1)");
        );
    );

    TEST
    (   "array equality -- test using == directly, since checkEquals can override.",
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) == array<int>({5, 4, 3, 2}), True);
        EXPECT_EQUAL(array<int>({5, 4, 3, 20}) == array<int>({5, 4, 3, 2}), False);
        // check shorter/bigger sizes:
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) == array<int>({5, 4, 3}), False);
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) == array<int>({5, 4, 3, 2, 1}), False);

        // and inequality, too:
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) != array<int>({5, 4, 3, 2}), False);
        EXPECT_EQUAL(array<int>({5, 4, 3, 20}) != array<int>({5, 4, 3, 2}), True);
        // check shorter/bigger sizes:
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) != array<int>({5, 4, 3}), True);
        EXPECT_EQUAL(array<int>({5, 4, 3, 2}) != array<int>({5, 4, 3, 2, 1}), True);
    );

    TEST
    (   "array iteration via for-loops",
        TEST
        (   "empty array iterates fine",
            array<int> Array;
            int I = 0;
            for (int &A : Array.values())
            {   ++I;
                LOG_ERR("UNEXPECTED " << A);
            }
            EXPECT_EQUAL(I, 0);
        );

        TEST
        (   "iteration via `values()`",
            array<int> Array({1, 5, 0, 4, 3, 1, 15});
            array<int> IterationResults;
            for (int A : Array.values())
            {   IterationResults.append(A);
            }
            EXPECT_EQUAL(IterationResults, Array);
        );

        TEST
        (   "iteration via `values()` gives const references if array is const.",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction logic

            int I = 0;
            for (const noisy &A : constant(Array).values())
            {   EXPECT_EQUAL(A.Value, I++);
            }
            EXPECT_EQUAL(I, 3);

            // Looping through a const Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        );

        TEST
        (   "mutating the array while iterating",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction noise

            for (noisy &A : Array.values())
            {   A.Value *= 5;
            }

            // Looping through a referenced Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array, array<noisy>({noisy(0), noisy(5), noisy(10)}));
        );

        TEST
        (   "mutating the array via elements()",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction noise

            index Index = 0;
            for (arrayElement<noisy &> Element : Array.elements())
            {   Element.Value.Value *= 5;
                EXPECT_EQUAL(Index++, Element.Index);
            }

            // Looping through a referenced Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array, array<noisy>({noisy(0), noisy(5), noisy(10)}));
        );

        TEST
        (   "iterating the array via constant().elements()",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction noise

            index Index = 0;
            for (arrayElement<const noisy &> Element : constant(Array).elements())
            {   EXPECT_EQUAL(Element.Value.Value, Index);
                EXPECT_EQUAL(Element.Index, Index++);
            }

            // Looping through a referenced Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array, array<noisy>({noisy(0), noisy(1), noisy(2)}));
        );
    );

    TEST
    (   "fixedCount array",
        TEST
        (   "can be created via array::fixedCount",
            auto Fixed = array<int>::fixedCount(10);
           
            // Default value is the the default-constructed value:
            for (int I = 0; I < 10; ++I)
            {   EXPECT_EQUAL(Fixed[I], 0);
            }

            EXPECT_EQUAL(Fixed.count(), 10);
            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_THROW(Fixed.count(8), ArrayFixedCountErrorMsg);
            EXPECT_THROW(Fixed.count(11), ArrayFixedCountErrorMsg);
        );

        TEST
        (   "can be created via array::fixCount",
            array<int> Fixed;
            Fixed.count(31);

            Fixed.fixCount();

            EXPECT_EQUAL(Fixed.count(), 31);
            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_THROW(Fixed.count(10), ArrayFixedCountErrorMsg);
            EXPECT_THROW(Fixed.count(100), ArrayFixedCountErrorMsg);
        );

        TEST
        (   "errors when append/reserve/remove/erase/insert/pop/access-over-bounds happens",
            auto Fixed = array<dbl>::fixedCount(7);
            Fixed[3] = 3000;

            Fixed.reserve(5); // Should be OK
            EXPECT_THROW(Fixed.reserve(10), ArrayFixedCountErrorMsg);

            EXPECT_THROW(Fixed.erase(0), ArrayFixedCountErrorMsg);
            Fixed.erase(1, 0); // Ok to erase 0 elements
            EXPECT_THROW(Fixed.erase(5, 2), ArrayFixedCountErrorMsg);

            EXPECT_THROW(Fixed.remove(0), ArrayFixedCountErrorMsg);

            EXPECT_THROW(Fixed.pop(), ArrayFixedCountErrorMsg);
            EXPECT_THROW(Fixed.append(5), ArrayFixedCountErrorMsg);
            EXPECT_THROW(Fixed[100], ArrayFixedCountErrorMsg);

            // No side effects:
            EXPECT_EQUAL(Fixed, array<dbl>({0, 0, 0, 3000, 0, 0, 0}));
        );

        TEST
        (   "can swap items, even sort",
            auto Fixed = array<int>::fixedCount(5);

            Fixed[0] = +100;
            Fixed[1] = -300;
            Fixed[2] = -500;
            Fixed[3] = +730;
            Fixed[4] = 1000;

            EXPECT_EQUAL(Fixed.swap(1, -301), -300);
            Fixed.swapIndices(4, 3);

            EXPECT_EQUAL(Fixed, array<int>({100, -301, -500, 1000, 730}));

            Fixed.sort();

            EXPECT_EQUAL(Fixed, array<int>({-500, -301, 100, 730, 1000}));
        );
        
        TEST
        (   "can grab items as long as they are not above bounds",
            auto Fixed = array<dbl>::fixedCount(11);

            Fixed[1] = 100.0;
            Fixed[5] = 500.0;
            Fixed[10] = 1010;

            EXPECT_EQUAL(Fixed, array<dbl>({0, 100, 0, 0, 0, 500, 0, 0, 0, 0, 1010}));

            EXPECT_EQUAL(Fixed[0], 0.0);
            EXPECT_EQUAL(Fixed[1], 100.0);
            EXPECT_EQUAL(Fixed[5], 500.0);
            EXPECT_EQUAL(Fixed[10], 1010.0);

            EXPECT_THROW(Fixed[11], ArrayFixedCountErrorMsg);
        );

        TEST
        (   "can get() values, even above bounds",
            auto Fixed = array<dbl>::fixedCount(3);

            *Fixed.get(0) = -0.5;
            *Fixed.get(1) = 1.0;
            *Fixed.get(2) = 1.75;

            EXPECT_EQUAL(Fixed, array<dbl>({-0.5, 1.0, 1.75}));

            EXPECT_POINTER_EQUAL(Fixed.get(0), -0.5);
            EXPECT_POINTER_EQUAL(Fixed.get(1), 1.0);
            EXPECT_POINTER_EQUAL(Fixed.get(2), 1.75);

            EXPECT_EQUAL(Fixed.get(3), Null);
            // TODO: EXPECT_THROW(*Fixed.get(3), "asdf");
        );

        TEST
        (   "FixedCount is sticky to variables when copied",
            auto Fixed = array<int>::fixedCount(8);
            auto NotFixed = array<int>({0,1,2,3,4,5,6,7});

            Fixed = NotFixed;

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(NotFixed.fixedCount(), False);
            EXPECT_EQUAL(Fixed, NotFixed);

            for (int I = 0; I < 8; ++I)
            {   Fixed[I] = 100 + I;
            }
            EXPECT_NOT_EQUAL(Fixed, NotFixed);

            NotFixed = Fixed;

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(NotFixed.fixedCount(), False);
            EXPECT_EQUAL(Fixed, NotFixed);
            EXPECT_EQUAL(NotFixed, array<int>({100, 101, 102, 103, 104, 105, 106, 107}));
        );

        TEST
        (   "fixedCount array may truncate when copying in other size array:",
            auto Fixed = array<int>({1,2,3,4});
            Fixed.fixCount();

            // too small:
            auto Source = array<int>({10, 20, 30});
            Fixed = Source;
            EXPECT_EQUAL(Fixed, array<int>({10, 20, 30, 0}));

            // too big:
            Source = array<int>({-1, -2, -3, -4, -5});
            Fixed = Source;
            EXPECT_EQUAL(Fixed, array<int>({-1, -2, -3, -4}));
        );

        TEST
        (   "fixedCount is sticky to variables when moved",
            auto Fixed = array<int>::fixedCount(8);
            auto NotFixed = array<int>({0,1,2,3,4,5,6,7});

            Fixed = std::move(NotFixed);

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(NotFixed.fixedCount(), False);
            EXPECT_EQUAL(Fixed, array<int>({0,1,2,3,4,5,6,7}));
            // moving NotFixed should reset the array:
            EXPECT_EQUAL(NotFixed, array<int>());

            NotFixed = std::move(Fixed);

            // Not strictly necessary, but something we do is reset the moved Fixed to not fixed:
            EXPECT_EQUAL(Fixed.fixedCount(), False);
            EXPECT_EQUAL(NotFixed.fixedCount(), False);
            EXPECT_EQUAL(NotFixed, array<int>({0,1,2,3,4,5,6,7}));
            // moving NotFixed should reset the array:
            EXPECT_EQUAL(Fixed, array<int>());
        );

        TEST
        (   "fixedCount array may truncate when moving in other size array:",
            auto Fixed = array<int>({1,2,3,4});
            Fixed.fixCount();

            // too small:
            auto Source = array<int>({10, 20, 30});
            Fixed = std::move(Source);
            EXPECT_EQUAL(Fixed, array<int>({10, 20, 30, 0}));

            // too big:
            Source = array<int>({-1, -2, -3, -4, -5});
            Fixed = std::move(Source);
            EXPECT_EQUAL(Fixed, array<int>({-1, -2, -3, -4}));
        );

        TEST
        (   "fixedCount is sticky to variables when swapped",
            auto Fixed = array<noisy>({noisy(-1), noisy(-2), noisy(-3)});
            Fixed.fixCount();
            auto NotFixed = array<noisy>({noisy(5), noisy(6), noisy(7)});
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // Ignore construction

            std::swap(Fixed, NotFixed);
            // No re-construction or moving at all of noisy instances:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(Fixed, array<noisy>({noisy(5), noisy(6), noisy(7)}));
            EXPECT_EQUAL(NotFixed, array<noisy>({noisy(-1), noisy(-2), noisy(-3)}));
            EXPECT_EQUAL(NotFixed.fixedCount(), False);
            // Ignore equality comparison constructions for next batch.
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));

            // Retrying in the other order:
            std::swap(NotFixed, Fixed);
            // No re-construction or moving at all of noisy instances:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(Fixed, array<noisy>({noisy(-1), noisy(-2), noisy(-3)}));
            EXPECT_EQUAL(NotFixed, array<noisy>({noisy(5), noisy(6), noisy(7)}));
            EXPECT_EQUAL(NotFixed.fixedCount(), False);

            // Try swapping with both fixed:
            auto AnotherFixed = array<noisy>::fixedCount(3);
            // Ignore constructions for next batch:
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy"));

            std::swap(Fixed, AnotherFixed);
            // No re-construction or moving at all of noisy instances:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Fixed.fixedCount(), True);
            EXPECT_EQUAL(Fixed, array<noisy>({noisy(), noisy(), noisy()}));
            EXPECT_EQUAL(AnotherFixed, array<noisy>({noisy(-1), noisy(-2), noisy(-3)}));
            EXPECT_EQUAL(AnotherFixed.fixedCount(), True);
        );

        TEST
        (   "errors when swapping non-compatible sized arrays if one (or both) is FixedCount",
            auto Fixed = array<int>::fixedCount(3);
            Fixed[1] = 101;
            auto NotFixed = array<int>({0,1,2,3,4});

            EXPECT_THROW(std::swap(Fixed, NotFixed), ArrayFixedCountErrorMsg);
            EXPECT_THROW(std::swap(NotFixed, Fixed), ArrayFixedCountErrorMsg);
            // no side effects:
            EXPECT_EQUAL(Fixed, array<int>({0, 101, 0}));

            NotFixed = array<int>({0,1});

            EXPECT_THROW(std::swap(Fixed, NotFixed), ArrayFixedCountErrorMsg);
            EXPECT_THROW(std::swap(NotFixed, Fixed), ArrayFixedCountErrorMsg);
            // no side effects:
            EXPECT_EQUAL(Fixed, array<int>({0, 101, 0}));

            auto AnotherFixed = array<int>::fixedCount(5);

            EXPECT_THROW(std::swap(Fixed, AnotherFixed), ArrayFixedCountErrorMsg);
            EXPECT_THROW(std::swap(AnotherFixed, Fixed), ArrayFixedCountErrorMsg);
            // no side effects:
            EXPECT_EQUAL(Fixed, array<int>({0, 101, 0}));
        );

        TEST
        (   "mutating the array while iterating",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            Array.fixCount();
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction noise

            for (noisy &A : Array.values())
            {   A.Value *= 5;
            }

            // Looping through a referenced Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array, array<noisy>({noisy(0), noisy(5), noisy(10)}));
        );

        TEST
        (   "mutating the array via elements()",
            array<noisy> Array({noisy(0), noisy(1), noisy(2)});
            Array.fixCount();
            ASSERT_STRING(TestPrintOutput.pull(), contains("noisy")); // ignore construction noise

            index Index = 0;
            for (arrayElement<noisy &> Element : Array.elements())
            {   Element.Value.Value *= 5;
                EXPECT_EQUAL(Index++, Element.Index);
            }

            // Looping through a referenced Array should make no copies:
            EXPECT_EQUAL(TestPrintOutput.pull(), "");

            EXPECT_EQUAL(Array, array<noisy>({noisy(0), noisy(5), noisy(10)}));
        );
    );

    TEST
    (   "arrayView",
        TEST
        (   "count() works for different sized elements",
            {   array<u8> Array({0, 1, 2, 3, 4});
                EXPECT_EQUAL(Array.view().count(), 5);
                EXPECT_EQUAL(constant(Array).view().count(), 5);
            }

            {   array<u16> Array({0, 1, 2, 3});
                EXPECT_EQUAL(Array.view().count(), 4);
                EXPECT_EQUAL(constant(Array).view().count(), 4);
            }

            {   array<u64> Array({0, 1, 2, 3, 4, 5, 6});
                EXPECT_EQUAL(Array.view().count(), 7);
                EXPECT_EQUAL(constant(Array).view().count(), 7);
            }
        );

        TEST
        (   "empty() works for different sized elements",
            {   array<u8> Array({0, 1, 2, 3, 4});
                EXPECT_EQUAL(Array.view().empty(), False);
                EXPECT_EQUAL(constant(Array).view().empty(), False);
                
                Array.clear();

                EXPECT_EQUAL(Array.view().empty(), True);
                EXPECT_EQUAL(constant(Array).view().empty(), True);
            }

            {   array<u64> Array({0, 1, 2, 3, 4, 5, 6});
                EXPECT_EQUAL(Array.view().empty(), False);
                EXPECT_EQUAL(constant(Array).view().empty(), False);
                
                Array.clear();

                EXPECT_EQUAL(Array.view().empty(), True);
                EXPECT_EQUAL(constant(Array).view().empty(), True);
            }
        )

        TEST
        (   "shiftView() works for different sized elements",
            TEST
            (   "works for non-const arrayViews:",
                {   array<u8> Array({0, 1, 2, 3, 4});
                    arrayView<u8> ArrayView = Array.view();

                    EXPECT_EQUAL(ArrayView.shiftView(), 0);
                    ArrayView.shiftView() *= 103;
                    EXPECT_EQUAL(ArrayView.shiftView(), 2);
                    ArrayView.shiftView() = 255;
                    EXPECT_EQUAL(ArrayView.shiftView(), 4);

                    EXPECT_THROW(ArrayView.shiftView(), ArrayViewEmptyMsg);

                    EXPECT_EQUAL(Array, array<u8>({0, 103, 2, 255, 4}));
                }

                {   array<i64> Array({-1, -2, -3});
                    arrayView<i64> ArrayView = Array.view();

                    ArrayView.shiftView() *= 103;
                    EXPECT_EQUAL(ArrayView.shiftView(), -2);
                    ArrayView.shiftView() = -300;

                    EXPECT_THROW(ArrayView.shiftView(), ArrayViewEmptyMsg);

                    EXPECT_EQUAL(Array, array<i64>({-103, -2, -300}));
                }
            );

            TEST
            (   "works for const arrayViews:",
                {   array<u8> Array({0, 1, 2, 3, 4});
                    arrayView<const u8> ArrayView = constant(Array).view();

                    EXPECT_EQUAL(ArrayView.shiftView(), 0);
                    EXPECT_EQUAL(ArrayView.shiftView(), 1);
                    EXPECT_EQUAL(ArrayView.shiftView(), 2);
                    EXPECT_EQUAL(ArrayView.shiftView(), 3);
                    const u8 &Reference = ArrayView.shiftView();
                    EXPECT_EQUAL(Reference, 4);

                    EXPECT_THROW(ArrayView.shiftView(), ArrayViewEmptyMsg);
                }

                {   array<i64> Array({-1, -2, -3});
                    arrayView<const i64> ArrayView = constant(Array).view();

                    EXPECT_EQUAL(ArrayView.shiftView(), -1);
                    EXPECT_EQUAL(ArrayView.shiftView(), -2);
                    EXPECT_EQUAL(ArrayView.shiftView(), -3);

                    EXPECT_THROW(ArrayView.shiftView(), ArrayViewEmptyMsg);
                }
            );
        );

        TEST
        (   "popView() works for different sized elements",
            TEST
            (   "works for non-const arrayViews:",
                {   array<u8> Array({0, 1, 2, 3, 4});
                    arrayView<u8> ArrayView = Array.view();

                    EXPECT_EQUAL(ArrayView.popView(), 4);
                    ArrayView.popView() *= 85;
                    EXPECT_EQUAL(ArrayView.popView(), 2);
                    ArrayView.popView() += 102;
                    EXPECT_EQUAL(ArrayView.popView(), 0);

                    EXPECT_THROW(ArrayView.popView(), ArrayViewEmptyMsg);

                    EXPECT_EQUAL(Array, array<u8>({0, 103, 2, 255, 4}));
                }

                {   array<i64> Array({-1, -2, -3});
                    arrayView<i64> ArrayView = Array.view();

                    ArrayView.popView() *= 103;
                    EXPECT_EQUAL(ArrayView.popView(), -2);
                    ArrayView.popView() = -11;

                    EXPECT_THROW(ArrayView.popView(), ArrayViewEmptyMsg);

                    EXPECT_EQUAL(Array, array<i64>({-11, -2, -309}));
                }
            );

            TEST
            (   "works for const arrayViews:",
                {   array<u8> Array({0, 1, 2, 3, 4});
                    arrayView<const u8> ArrayView = constant(Array).view();

                    EXPECT_EQUAL(ArrayView.popView(), 4);
                    EXPECT_EQUAL(ArrayView.popView(), 3);
                    EXPECT_EQUAL(ArrayView.popView(), 2);
                    EXPECT_EQUAL(ArrayView.popView(), 1);
                    const u8 &Reference = ArrayView.popView();
                    EXPECT_EQUAL(Reference, 0);

                    EXPECT_THROW(ArrayView.popView(), ArrayViewEmptyMsg);
                }

                {   array<i64> Array({-1, -2, -3});
                    arrayView<const i64> ArrayView = constant(Array).view();

                    EXPECT_EQUAL(ArrayView.popView(), -3);
                    EXPECT_EQUAL(ArrayView.popView(), -2);
                    EXPECT_EQUAL(ArrayView.popView(), -1);

                    EXPECT_THROW(ArrayView.popView(), ArrayViewEmptyMsg);
                }
            );
        );
        // TODO: shift+popView tests
    );
}
#endif

TMVB
