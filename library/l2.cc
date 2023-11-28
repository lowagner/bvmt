#include "l2.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

l2Owned l2::owned(size2i Size)
{   return l2Owned(Size);
}
l2Borrowed l2::borrowed(bvmt::texture &Borrowed_Texture)
{   return l2Borrowed(Borrowed_Texture);
}

textureBatch l2::batch()
{   return texture()->batch();
}

void l2::writeToRow(const char *Chars)
{   if (Chars[0] == 0) return;
    textureBatch Batch = batch();
    // TODO: calculate Offset from Font size and Texture size.
}

l2Owned::l2Owned(size2i Size) : l2(), Texture(Size)
{}

texture *l2Owned::texture() { return &Texture; }

l2Borrowed::l2Borrowed(bvmt::texture &Borrowed_Texture)
{   Texture = &Borrowed_Texture;
}

texture *l2Borrowed::texture() { return Texture; }

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
