#pragma once

#include "dimensions.h"

#include "../core/types.h"

NAMESPACE

class l2;
class l3;

typedef size2<i32> windowResolution;

struct windowTexture {
    windowTexture(windowResolution Resolution);
    ~windowTexture();
private:
    u8 Data[20 * 2 + 4];
};

struct window {
    static constexpr windowResolution DefaultWindowResolution
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
    windowResolution Resolution = DefaultWindowResolution;
    // The L3 texture is drawn first.
    windowTexture *TextureL3;
    // The L2 texture is drawn second, i.e., as a HUD, in case of anything in L3.
    windowTexture *TextureL2;

    friend class l2;
};

ECAPSEMAN
