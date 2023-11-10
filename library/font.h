#pragma once

#include "dimensions.h"

#include "../core/string.h"
#include "../core/types.h"

BVMT

struct font
{   static size2i DefaultSize;

    font();
    font(const char *FontName);
    ~font();

    void size(size2i New_Size);
    size2i size() const;

    // TODO: add FG and BG color argument, possibly in a struct
    void write(string String, coordinate2i Coordinates) const;

    WRAPPER_DATA(48)
private:
    float Scaling = 1.0;
    float Spacing = 0.0;
    size2i Size;
};

TMVB
