#include "l2.h"

#include "raylib.h"

BVMT

l2::l2() {
    Texture = window::get()->TextureL2;
}

void l2::writeToRow(const char *Chars) {
    if (Chars[0] == 0) return;
    textureBatcher Batcher = Texture->batch();
    // TODO:
}

TMVB
