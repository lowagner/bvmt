#include "window.h"

#include "raylib.h"

BVMT

WRAPPER(windowTexture, RenderTexture2D)

windowTexture::windowTexture(windowResolution Resolution) {
    unwrap(This) = LoadRenderTexture(Resolution.Width, Resolution.Height);
}

windowTexture::~windowTexture() {
    UnloadRenderTexture(unwrap(This)); 
}

SINGLETON_CC(window, {
    InitWindow(Resolution.Width, Resolution.Height, "bvmt");
    TextureL3 = new windowTexture(DefaultWindowResolution);
    TextureL2 = new windowTexture(DefaultWindowResolution);
})

window::~window() {
    DELETE(TextureL2);
    DELETE(TextureL3);
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

TMVB
