#include "window.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

#include <algorithm> // std::fill

BVMT

WRAPPER(windowTexture, RenderTexture2D)

windowTexture::windowTexture()
{   std::fill(std::begin(Data), std::end(Data), 0);
}

windowTexture::windowTexture(windowResolution Resolution)
{   unwrap(This) = LoadRenderTexture(Resolution.Width, Resolution.Height);
}

textureBatcher windowTexture::batch()
{   return pushPop(This);
}

void windowTexture::firstPush()
{   BeginTextureMode(unwrap(This));        
}

void windowTexture::lastPop()
{   EndTextureMode();
}

windowTexture::~windowTexture()
{   UnloadRenderTexture(unwrap(This)); 
}

SINGLETON_CC(window,
{   InitWindow(Resolution.Width, Resolution.Height, "bvmt");
    TextureL3 = new windowTexture(DefaultResolution);
    TextureL2 = new windowTexture(DefaultResolution);
})

window::~window()
{   DELETE(TextureL2);
    DELETE(TextureL3);
    CloseWindow();
}

bool window::resolution(windowResolution New_Resolution)
{   if (New_Resolution.Width < 64 or New_Resolution.Height < 64)
    {   return False;
    }
    if (New_Resolution.Width > 3840 or New_Resolution.Height > 2160)
    {   return False;
    }
    Resolution = New_Resolution;
    // TODO: resize textures
    return True;
}

windowResolution window::resolution() const
{   return Resolution;
}

#ifndef NDEBUG
void test__library__window()
{   TEST
    (   "test stuff",
        // TODO
        {}
    );
}
#endif

TMVB
