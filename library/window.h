#pragma once

#include "dimensions.h"
#include "push-pop.h"

#include "../core/types.h"

BVMT

class l2;
class l3;
class windowTexture;

typedef size2<i32> windowResolution;
typedef pushPop<windowTexture> textureBatcher;

struct windowTexture
{   // A texture that the window can draw.
    windowTexture(windowResolution Resolution);
    // This is a null texture that is ineligible to be drawn to.
    windowTexture();
    ~windowTexture();

    UNCOPYABLE_CLASS(windowTexture)
    UNMOVABLE_CLASS(windowTexture) // for pushPop

    // You can draw or write to this texture while this return value
    // is in scope; when it descopes it will automatically close
    // the write.  This is safe to call/nest multiple times for the
    // same texture.
    textureBatcher batch();

    WRAPPER_DATA(20 * 2 + 4)
    PUSHER_POPPER_H()
};

struct window
{   // Represents a physical window for the program.
    static constexpr windowResolution DefaultResolution
        =   windowResolution{.Width = 800, .Height = 450};
    SINGLETON_H(window)
    ~window();

    // Sets the width and height of the interior drawing region, in pixels.
    // Returns true if successful.
    bool resolution(windowResolution New_Resolution);
    // Gets the width and height of the interior drawing region, in pixels.
    windowResolution resolution() const;

    // TODO: `void clear(color Color)`
    // TODO: probably need a `void needsRedraw()`
private:
    windowResolution Resolution = DefaultResolution;
    // The L3 texture is drawn first.
    windowTexture *TextureL3;
    // The L2 texture is drawn second, i.e., as a HUD, in case of anything in L3.
    windowTexture *TextureL2;

    friend class l2;
};

TMVB
