#pragma once

#include "dimensions.h"
#include "font.h"
#include "texture.h"

#include "../core/types.h"

BVMT

class l2Owned;
class l2Borrowed;

struct l2
{   // Class that wraps drawing to a texture for writing words, etc., to the screen.
protected:
    // Not constructible directly, use one of the `l2::owned` or `l2::borrowed` static methods.
    l2() = default;

public:
    static l2Owned owned(size2i Size);
    static l2Borrowed borrowed(bvmt::texture &Borrowed_Texture);

    // TODO: `rgba Foreground;`
    // TODO: `rgba Background;`
    // Current drawing position, in terms of rows/columns (e.g., like a terminal).
    index2i Position;
    bvmt::font *Font = Null;

    // Creates a batch operation for writing to the underlying texture
    // multiple times.  If you are doing many operations, prefer calling
    // this first.  This is safe to call multiple times, even nested,
    // but probably not from separate threads.  Note that all `l2`s
    // spawned from `window` share a common texture to write to.
    // Note this batch method is automatically called for you when doing
    // `window::l2(fn<void(l2)>)``.
    textureBatch batch();
    void writeToRow(const char *Chars);

    // TODO: `bvmt::coordinates coordinates(index2i Other_Position) const`
    //       to help with drawing to a specific region close to some position.
private:
    virtual bvmt::texture *texture() = 0;
};

// TODO: when drawing an l2 in an l3, we need the l3 to own the texture
// and draw it into the scene.

struct l2Owned : public l2
{   // l2 class which owns its texture.

    // TODO: we need a way of drawing this l2 to the window.
    //       probably can add a `draw(coordinates, l2 *)` method to `l2`.
    //       as well as a `draw(l2 *)` for use with the parent l2's Position.
    // TODO: technically we can copy, but we need to do it carefully.
    // e.g., create a new `l2Owned` and render the old texture to its texture.
    UNCOPYABLE_CLASS(l2Owned)
    UNMOVABLE_CLASS(l2Owned)

    l2Owned(size2i Size);

private:
    bvmt::texture *texture() override;
    bvmt::texture Texture;

    friend class l2;
};

struct l2Borrowed : public l2
{   // l2 class which borrows its texture.
    UNCOPYABLE_CLASS(l2Borrowed)
    UNMOVABLE_CLASS(l2Borrowed)

    l2Borrowed(bvmt::texture &Borrowed_Texture);

private:
    bvmt::texture *texture() override;
    bvmt::texture *Texture = Null;

    friend class l2;
};

TMVB
