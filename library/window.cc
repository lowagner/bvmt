#include "window.h"

#include "l2.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

SINGLETON_CC(window,
{   InitWindow(Resolution.Width, Resolution.Height, "bvmt");
    TextureL3 = new texture(DefaultResolution);
    TextureL2 = new texture(DefaultResolution);
})

window::~window()
{   DELETE(TextureL2);
    DELETE(TextureL3);
    CloseWindow();
}

void window::l2(fn<void(bvmt::l2 *)> L2Modifier_fn)
{   l2Borrowed L2(*TextureL2);
    textureBatch Batch = L2.batch();
    L2Modifier_fn(&L2);
}

bool window::resolution(size2i New_Resolution)
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

size2i window::resolution() const
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
