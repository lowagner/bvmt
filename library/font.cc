#include "font.h"

#ifndef NDEBUG
#include "../core/error.h"
#endif

#include "raylib.h"

BVMT

WRAPPER(font, Font)

fontSize font::DefaultSize = fontSize{.Width = 15, .Height = 20};

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

void font::size(fontSize New_Size)
{   // TODO: figure out Multiplier & Spacing for desired pixel width/height
    Size = New_Size;
}

fontSize font::size() const
{   return Size;
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
