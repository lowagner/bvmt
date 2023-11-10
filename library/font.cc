#include "font.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

WRAPPER(font, Font)

size2i font::DefaultSize = size2i{.Width = 15, .Height = 20};

font::font()
:   font("../shared/resources/sono/Sono-Medium.ttf")
{}

font::font(const char *FontName)
{   unwrap(This) = LoadFont(FontName);
    size(DefaultSize);
}

font::~font()
{   UnloadFont(unwrap(This));
}

void font::size(size2i New_Size)
{   // We'll figure out Scaling & Spacing for the desired pixel width/height.
    // TODO: Use New_Size.Height as the line height, but
    //      try to add 2/1 pixels to top/bottom if possible.
    Font RaylibFont = unwrap(This);
    /*
    // Get width from `W`
    int WIndex = GetGlyphIndex(RaylibFont, 'W');
    GlyphInfo *WGlyph = RaylibFont.Glyphs + WIndex;
    // Scaling * (Glyph.AdvanceX || Rectangle.Width) + Spacing
    int Width = WGlyph->advanceX ? WGlyph->advanceX : RaylibFont.recs[WIndex].width;
    // Get height from `q`
    GlyphInfo *QGlyph = RaylibFont.Glyphs + GetGlyphIndex(RaylibFont, 'q');
    int Height = QGlyph.height
    */
    Size = New_Size;
}

size2i font::size() const
{   return Size;
}

void font::write(string String, coordinate2i Coordinates) const
{   Font RaylibFont = unwrap(This);
    DrawTextEx
    (   RaylibFont,
        String.chars(),
        Vector2{Coordinates.X, Coordinates.Y},
        Scaling * RaylibFont.baseSize,
        Spacing,
        WHITE
    );
}

#ifndef NDEBUG
void test__library__font()
{   TEST
    (   "test stuff",
        // TODO
        {}
    );
}
#endif

TMVB
