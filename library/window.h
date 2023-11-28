#pragma once

#include "dimensions.h"
#include "push-pop.h"
#include "texture.h"

#include "../core/pointer.h"
#include "../core/types.h"

BVMT

class l2;
class l3;
class window;

typedef pushPop<window> windowDraw;

struct window
{   // Represents a physical window for the program.
    static constexpr size2i DefaultResolution
        =   size2i{.Width = 800, .Height = 450}
    ;
    SINGLETON_H(window)
    ~window();

    // Call to draw to the window.  Let the returned value
    // get descoped before you try to draw again.
    windowDraw draw();

    // TODO: add a `windowDraw draw(texture, offset2i)` method.

    // Don't resize this window inside the callback.
    void l2(fn<void(bvmt::l2 *)> L2Modifier_fn);

    // Sets the width and height of the interior drawing region, in pixels.
    // Returns true if successful.
    bool resolution(size2i New_Resolution);
    // Gets the width and height of the interior drawing region, in pixels.
    size2i resolution() const;

    // TODO: `void clear(color Color)`
    // TODO: probably need a `void needsRedraw()`

    // TODO: add Font or DefaultFont to window

    PUSHER_POPPER_H()
private:
    size2i Resolution = DefaultResolution;
    // The L3 texture is drawn first.
    pointer<texture> TextureL3;
    // The L2 texture is drawn second, i.e., as a HUD, in case of anything in L3.
    pointer<texture> TextureL2;

    void draw(const texture &The_Texture);
};

TMVB
