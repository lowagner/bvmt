#pragma once

#include "../core/types.h"

BVMT

template <class t>
struct coordinate2
{   t X = t();
    t Y = t();
};

template <class t>
struct index2
{   t Column = t();
    t Row = t();
};

template <class t>
struct size2
{   t Width = t();
    t Height = t();
};

typedef coordinate2<i32> coordinate2i;
typedef index2<i32> index2i;
typedef size2<i32> size2i;

TMVB
