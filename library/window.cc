#include "window.h"

#include "l2.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

WRAPPER(texture, RenderTexture2D)

SINGLETON_CC(window,
{   InitWindow(Resolution.Width, Resolution.Height, "bvmt");
    TextureL3 = pointer<texture>::deleteOnDescope(new texture(DefaultResolution));
    TextureL2 = pointer<texture>::deleteOnDescope(new texture(DefaultResolution));
})

window::~window()
{   CloseWindow();
}

windowDraw window::draw()
{   return pushPop(This);
}

void window::firstPush()
{   BeginDrawing();
    // TODO: change to a desired color.
    ClearBackground(RAYWHITE);
    draw(*TextureL3); // L3 goes first so it's drawn behind everything
}

void window::lastPop()
{   draw(*TextureL2); // L2 goes on top (e.g., for HUD)
    EndDrawing();
}

void window::draw(const texture &The_Texture)
{   RenderTexture2D RaylibTexture = unwrap(The_Texture);
    const float virtualRatio = (float)Resolution.Width / (float)RaylibTexture.texture.width;
    DrawTexturePro
    (   RaylibTexture.texture,
        // Negate Source rectangle height for OpenGL reasons:
        Rectangle {0.0f, 0.0f, RaylibTexture.texture.width, -RaylibTexture.texture.height},
        // Destination:
        Rectangle
        {   -virtualRatio,
            -virtualRatio,
            GetScreenWidth() + (virtualRatio * 2),
            GetScreenHeight() + (virtualRatio * 2),
        },
        Vector2 {0.0f, 0.0f}, // Origin
        0.0f,
        WHITE
    );
}

void window::l2(fn<void(bvmt::l2 *)> L2Modifier_fn)
{   l2Borrowed L2(*TextureL2);
    textureBatch Batch = L2.batch();
    L2Modifier_fn(&L2);
}

bool window::resolution(size2i New_Resolution)
{   if (PushCount != 0)
    {   LOG_ERR("attempted to change window resolution during draw phase");
        return False;
    }
    if (New_Resolution.Width < 64 or New_Resolution.Height < 64)
    {   return False;
    }
    if (New_Resolution.Width > 3840 or New_Resolution.Height > 2160)
    {   return False;
    }
    if (New_Resolution == resolution())
    {   return True;
    }

    // Resize the textures:
    auto New_L2 = pointer<texture>::deleteOnDescope(new texture(New_Resolution));
    // TODO: New_L2->draw(*TextureL2)
    auto New_L3 = pointer<texture>::deleteOnDescope(new texture(New_Resolution));
    // TODO: New_L3->draw(*TextureL3)
    std::swap(New_L2, TextureL2);
    std::swap(New_L3, TextureL3);

    Resolution = New_Resolution;
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
