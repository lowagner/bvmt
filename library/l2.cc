#include "l2.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

l2::l2()
{   Texture = &window::get()->TextureL2;
}

textureBatch l2::batch()
{   return (*Texture)->batch();
}

void l2::writeToRow(const char *Chars)
{   if (Chars[0] == 0) return;
    textureBatch Batch = batch();
    // TODO:
}

#ifndef NDEBUG
void test__library__l2()
{   TEST
    (   "test stuff",
        // TODO
        {}
    );
}
#endif
TMVB
