#include "push-pop.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

BVMT

#ifndef NDEBUG
struct pushPopLogger 
{   int PushCount = 0;
    void firstPush()
    {   std::cout << "firstPush!";
    }
    void lastPop()
    {   std::cout << "lastPop!";
    }
};

typedef pushPop<pushPopLogger> testPushPop;

void test__library__push_pop()
{   TEST
    (   "increments in constructor and decrements in destructor",
        pushPopLogger Logger;
        {   testPushPop PushPop(Logger);
            EXPECT_EQUAL(Logger.PushCount, 1);
            EXPECT_EQUAL(TestPrintOutput.pull(), "firstPush!");
            {   testPushPop PushPop(Logger);
                EXPECT_EQUAL(Logger.PushCount, 2);
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
                {   testPushPop PushPop(Logger);
                    EXPECT_EQUAL(Logger.PushCount, 3);
                    EXPECT_EQUAL(TestPrintOutput.pull(), "");
                }
                EXPECT_EQUAL(Logger.PushCount, 2);
                EXPECT_EQUAL(TestPrintOutput.pull(), "");
            }
            EXPECT_EQUAL(Logger.PushCount, 1);
            EXPECT_EQUAL(TestPrintOutput.pull(), "");
        }
        EXPECT_EQUAL(Logger.PushCount, 0);
        EXPECT_EQUAL(TestPrintOutput.pull(), "lastPop!");
    );
}
#endif

TMVB
