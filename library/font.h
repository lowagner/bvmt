#pragma once

#include "dimensions.h"

#include "../core/types.h"

BVMT

typedef size2<i32> fontSize;

struct font
{   static fontSize DefaultSize;

    font();
    font(const char *FontName);
    ~font();

    void size(fontSize New_Size);
    fontSize size() const;

    // TODO: add a `drawText` method with arbitrary pixel coordinates/offsets.
    //      inside L2, we'll specify the pixel offsets based on line height,
    //      but here it can be whatever.

    WRAPPER_DATA(48)
private:
    float Scaling = 1.0;
    float Spacing = 0.0;
    fontSize Size;
};

TMVB
