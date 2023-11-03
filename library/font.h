#pragma once

#include "../core/types.h"

NAMESPACE

struct fontData { u8 Data[48]; };

// TODO: should this be a singleton or some sort of font map so we don't
//       constantly load/unload fonts?  or we can just pass around, e.g., `L2.draw(const font &F, ...)`
struct font {
    font();
    font(const char *FontName);
    ~font();

private:
    fontData Data;
};

ECAPSEMAN
