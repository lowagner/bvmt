#include "l2.h"

#include "raylib.h"

BVMT

l2::l2() {
    Texture = &window::get()->TextureL2;
}

textureBatcher l2::batch() {
    return Texture->batch();
}

void l2::writeToRow(const char *Chars) {
    if (Chars[0] == 0) return;
    textureBatcher Batcher = batch();
    // TODO:
}

TMVB
