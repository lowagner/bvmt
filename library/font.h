#pragma once

#include "../core/types.h"

NAMESPACE

struct fontData { u8 Data[48]; };

struct font {
    fontData Data;

    font();
    font(const char *FontName);
};

ECAPSEMAN
