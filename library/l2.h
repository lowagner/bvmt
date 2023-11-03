#pragma once

#include "dimensions.h"
#include "font.h"
#include "window.h"

#include "../core/types.h"

BVMT

typedef coordinates2<i32> l2Position;

struct l2 {
    const font *Font = Null;
    l2Position Position;
    // TODO: `color Color;`

    l2();

    void writeToRow(const char *Chars);
private:
    // This is a pointer because the window can change the texture
    // when changing resolution, and we want to grab the updated version.
    windowTexture *Texture;
};

// TODO: when drawing an l2 in an l3, we need the l3 to own the texture
// and draw it into the scene.

TMVB
