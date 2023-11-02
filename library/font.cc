#include "font.h"

#include "raylib.h"

NAMESPACE

WRAPPER(fontData, Font)

font::font() {
    unwrap(Data) = LoadFont("../shared/resources/sono/Sono-Medium.ttf");
}

font::font(const char *FontName) {
    unwrap(Data) = LoadFont(FontName);
}

ECAPSEMAN
