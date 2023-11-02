#pragma once

#include "../core/types.h"

NAMESPACE

struct fontData { u8 Data[48]; };

struct font {
    font();
    font(const char *FontName);
    ~font();

private:
    fontData Data;
};

ECAPSEMAN
