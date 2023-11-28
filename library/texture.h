#pragma once

#include "dimensions.h"
#include "push-pop.h"

#include "../core/types.h"

BVMT

struct texture;

typedef pushPop<texture> textureBatch;

struct texture 
{   // A texture that the window can draw.
    texture(size2i Size);
    // This is a null texture that is ineligible to be drawn to.
    texture();
    ~texture();

    // TODO: add `i32 width() const` and height methods, via `unwrap(This).texture.width`

    UNCOPYABLE_CLASS(texture)
    UNMOVABLE_CLASS(texture) // for pushPop

    // You can draw or write to this texture while this return value
    // is in scope; when it descopes it will automatically close
    // the write.  This is safe to call/nest multiple times for the
    // same texture.
    textureBatch batch();

    WRAPPER_DATA(20 * 2 + 4)
    PUSHER_POPPER_H()
};

TMVB
