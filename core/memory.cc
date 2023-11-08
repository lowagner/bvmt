#include "memory.h"

#ifndef NDEBUG
#include "array.h"
#include "string.h"
#endif

BVMT

#ifndef NDEBUG
using test::noisy;
using namespace memory;
void test__core__memory()
{   TEST
    (   "allocate + deallocate works with primitive type",
        i64 *Ints = allocate<i64>(10L);
        string PrintOutput = TestPrintOutput.pull();
        array<stringView> AllocateIo = PrintOutput.view().split('|');
        EXPECT_EQUAL(AllocateIo[0], "allocate<8x>(10)");

        deallocate(Ints);
        EXPECT_EQUAL
        (   TestPrintOutput.pull(),
            string("deallocate(") + AllocateIo[1] + ")"
        );
    );

    // TODO: reallocate
}
#endif

TMVB
