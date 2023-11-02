#pragma once

#include "dimensions.h"

#include "../core/types.h"

NAMESPACE

typedef size2<i32> windowResolution;

struct window {
    SINGLETON_H(window)
    ~window();

    // Sets the width and height of the interior drawing region, in pixels.
    // Returns true if successful.
    bool resolution(windowResolution New_Resolution);
    // Gets the width and height of the interior drawing region, in pixels.
    windowResolution resolution() const;

private:
    windowResolution Resolution = windowResolution{.Width = 800, .Height = 450};
};

ECAPSEMAN
