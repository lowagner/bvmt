#pragma once

#include "../core/types.h"

BVMT

template <class t>
struct pushPop
{   pushPop(t &_Data)
    :   Data(&_Data)
    {   if (++Data->PushCount == 1)
        {   Data->firstPush();
        }
    }

    ~pushPop()
    {   if (--Data->PushCount == 0)
        {   Data->lastPop();
        }
    }

private:
    t *Data;
};

TMVB
