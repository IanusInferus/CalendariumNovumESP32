#pragma once

#include <cstdint>

struct GlyphDescription
{
    std::uint16_t Unicode;
    std::uint16_t ImagePosX;
    std::uint16_t ImagePosY;
    std::uint8_t ImageGlyphWidth;
    std::uint8_t ImageGlyphHeight;
    std::uint8_t ImageGlyphOffsetX;
    std::uint8_t ImageGlyphOffsetY;
    std::uint8_t GlyphWidth;
    std::uint8_t GlyphHeight;
};

const int font_bitmap_width = 2048;
const int font_bitmap_height = 960;
const int font_glyph_width = 16;
const int font_glyph_height = 16;
extern const unsigned char font_bitmap[245760];
extern const GlyphDescription font_bitmap_desc[7580];
