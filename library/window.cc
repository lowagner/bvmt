#include "window.h"

#include "raylib.h"

NAMESPACE

SINGLETON_CC(window, {
    InitWindow(Resolution.Width, Resolution.Height, "bvmt");
})

window::~window() {
    CloseWindow();
}

bool window::resolution(windowResolution New_Resolution) {
    if (New_Resolution.Width < 64 or New_Resolution.Height < 64) {
        return False;
    }
    if (New_Resolution.Width > 3840 or New_Resolution.Height > 2160) {
        return False;
    }
    Resolution = New_Resolution;
    // TODO: resize textures
    return True;
}

windowResolution window::resolution() const {
    return Resolution;
}

ECAPSEMAN
