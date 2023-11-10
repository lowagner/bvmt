#pragma once

#include "dimensions.h"
#include "font.h"
#include "texture.h"
#include "window.h"

#include "../core/types.h"

BVMT

typedef coordinates2<i32> l2Position;

struct l2
{   const font *Font = Null;
    l2Position Position;
    // TODO: `rgba Foreground;`
    // TODO: `rgba Background;`

    l2();

    // Creates a batch operation for writing to the underlying texture
    // multiple times.  If you are doing many operations, prefer calling
    // this first.  This is safe to call multiple times, even nested,
    // but probably not from separate threads.  Note that all `l2`s
    // spawned from `window::l2()` share a common texture to write to.
    textureBatch batch();
    void writeToRow(const char *Chars);
private:
    // TODO: l2Position Min, Max
    // This is a pointer-pointer because the window can change the texture
    // when changing resolution, and we want to grab the updated version.
    texture **Texture;
};

// TODO: when drawing an l2 in an l3, we need the l3 to own the texture
// and draw it into the scene.

TMVB
