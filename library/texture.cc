#include "texture.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

#include <algorithm> // std::fill

BVMT

WRAPPER(texture, RenderTexture2D)

texture::texture()
{   std::fill(std::begin(Data), std::end(Data), 0);
}

texture::texture(size2i Size)
{   unwrap(This) = LoadRenderTexture(Size.Width, Size.Height);
}

textureBatch texture::batch()
{   return pushPop(This);
}

void texture::firstPush()
{   BeginTextureMode(unwrap(This));        
}

void texture::lastPop()
{   EndTextureMode();
}

texture::~texture()
{   UnloadRenderTexture(unwrap(This)); 
}

#ifndef NDEBUG
void test__library__texture()
{   TEST
    (   "test stuff",
        // TODO
        {}
    );
}
#endif

TMVB
