#include "Graphics.h"

#include "Font.h"
#include "StringUtilities.h"

#include <Arduino.h>
#include "components/epd/buff.h" // POST request data accumulator
#include "components/epd/epd.h"  // e-Paper driver

#include <cstring>
#include <algorithm>

namespace
{

unsigned char Buffer[400 * 300 / 8];

}

int Graphics::GetWidth()
{
    return 400;
}

int Graphics::GetHeight()
{
    return 300;
}

void Graphics::Init()
{
    EPD_initSPI();
}

void Graphics::Clear()
{
    std::memset(Buffer, 0, sizeof(Buffer) * sizeof(unsigned char));
}

void Graphics::DrawRectangle(int x, int y, int width, int height)
{
    if ((width <= 0) || (height <= 0)) { return; }
    auto BufferWidth = GetWidth();
    auto xl = std::max(x, 0);
    auto xu = std::min(x + width - 1, BufferWidth - 1);
    auto yl = std::max(y, 0);
    auto yu = std::min(y + height - 1, GetHeight() - 1);
    for (int j = yl; j <= yu; j += 1)
    {
        for (int i = xl; i <= xu; i += 1)
        {
            Buffer[(i + j * BufferWidth) / 8] = Buffer[(i + j * BufferWidth) / 8] | (1 << (7 - (i % 8)));
        }
    }
}

void Graphics::DrawRectangleFromImage(int x, int y, int width, int height, const unsigned char* image, int imageWidth, int imageHeight, int imagePosX, int imagePosY)
{
    if ((width <= 0) || (height <= 0)) { return; }
    auto BufferWidth = GetWidth();

    auto xlSource = std::max(imagePosX, 0);
    auto xuSource = std::min(imagePosX + width - 1, imageWidth - 1);
    auto ylSource = std::max(imagePosY, 0);
    auto yuSource = std::min(imagePosY + height - 1, imageHeight - 1);

    auto xl = std::max(xlSource - imagePosX + x, 0);
    auto xu = std::min(xuSource - imagePosX + x, BufferWidth - 1);
    auto yl = std::max(ylSource - imagePosY + y, 0);
    auto yu = std::min(yuSource - imagePosY + y, GetHeight() - 1);

    for (int j = yl; j <= yu; j += 1)
    {
        int jSource = j - y + imagePosY;
        for (int i = xl; i <= xu; i += 1)
        {
            int iSource = i - x + imagePosX;
            int c = (image[(iSource + jSource * imageWidth) / 8] >> (iSource % 8)) & 1;
            Buffer[(i + j * BufferWidth) / 8] = Buffer[(i + j * BufferWidth) / 8] | (c << (7 - (i % 8)));
        }
    }
}

void Graphics::DrawText(int x, int y, const char* text)
{
    auto u32Text = utf8ToUtf32(text);

    int xCurrent = x;
    int yCurrent = y;
    for (auto c : u32Text)
    {
        //Serial.println("Draw: " + String(c));
        if (c >= 0x10000)
        {
            xCurrent += font_glyph_width;
            continue;
        }

        GlyphDescription d = {};
        d.Unicode = static_cast<std::uint16_t>(c);
        auto range = std::equal_range(std::begin(font_bitmap_desc), std::end(font_bitmap_desc), d, [](const GlyphDescription& lhs, const GlyphDescription& rhs) -> bool
        {
            return lhs.Unicode < rhs.Unicode;
        });
        if (range.first == range.second)
        {
            Serial.println("NoGlyph: " + String(c));
            xCurrent += font_glyph_width;
            continue;
        }
        else
        {
            d = *range.first;
        }

        //Serial.println(("Glyph: " + std::to_string(static_cast<int>(c)) + " " + std::to_string(xCurrent) + " " + std::to_string(yCurrent) + " " + std::to_string(d.GlyphWidth) + " " + std::to_string(d.GlyphHeight)).c_str());
        DrawRectangleFromImage(xCurrent, yCurrent, d.GlyphWidth, d.GlyphHeight, font_bitmap, font_bitmap_width, font_bitmap_height, d.ImagePosX + d.ImageGlyphOffsetX, d.ImagePosY + d.ImageGlyphOffsetY);
        xCurrent += d.GlyphWidth;
    }
}

void Graphics::Flush()
{
    //EPD_Init_4in2();
    EPD_Reset();
    
    EPD_SendCommand(0x01);//POWER_SETTING
    EPD_SendData(0x03);   // VDS_EN, VDG_EN
    EPD_SendData(0x00);   // VCOM_HV, VGHL_LV[1], VGHL_LV[0]
    EPD_SendData(0x2F);   // VDH
    EPD_SendData(0x2F);   // VDL
    EPD_SendData(0xFF);   // VDHR
    
    EPD_Send_3(0x06, 0x17, 0x17, 0x17);//BOOSTER_SOFT_START
    EPD_SendCommand(0x04);//POWER_ON
    EPD_WaitUntilIdle();
    
    EPD_Send_2(0x00, 0xBF, 0x0B);//PANEL_SETTING: // KW-BF   KWR-AF  BWROTP 0f
    EPD_Send_1(0x30, 0x3C);//PLL_CONTROL: 3A 100HZ, 29 150Hz, 39 200HZ, 31 171HZ

    EPD_Send_4(0x61, 1, 144, 1, 44);// RESOLUTION_SETTING: HI(W), LO(W), HI(H), LO(H)  
    EPD_Send_1(0x82, 0x12);// VCM_DC_SETTING                   
    EPD_Send_1(0x50, 0x97);// VCOM_AND_DATA_INTERVAL_SETTING: VBDF 17|D7 VBDW 97  VBDB 57  VBDF F7  VBDW 77  VBDB 37  VBDR B7

    EPD_lut(0x20,44,&lut_dc_4in2[0]);// LUT_FOR_VCOM
    EPD_lut(0x21,42,&lut_ww_4in2[0]);// LUT_WHITE_TO_WHITE   
    EPD_lut(0x22,42,&lut_bw_4in2[0]);// LUT_BLACK_TO_WHITE
    EPD_lut(0x23,42,&lut_wb_4in2[0]);// LUT_WHITE_TO_BLACK
    EPD_lut(0x24,42,&lut_bb_4in2[0]);// LUT_BLACK_TO_BLACK

    EPD_SendCommand(0x10);//DATA_START_TRANSMISSION_1  
    delay(2);
    for(int i = 0; i < 400*300/8; i++)EPD_SendData(0xFF);//Red channel

    EPD_SendCommand(0x13);//DATA_START_TRANSMISSION_2
    delay(2);

    // Commit Graphics Buffer
    for (int k = 0; k < sizeof(Buffer); k += 1)
    {
        EPD_SendData(~Buffer[k]);
    }

    //EPD_showB();

    // Refresh
    EPD_SendCommand(0x12);// DISPLAY_REFRESH
    delay(100);
    EPD_WaitUntilIdle();

    // Sleep
    EPD_Send_1(0x50, 0x17);// VCOM_AND_DATA_INTERVAL_SETTING
    EPD_Send_1(0x82, 0x00);// VCM_DC_SETTING_REGISTER, to solve Vcom drop
    EPD_Send_4(0x01, 0x02, 0x00, 0x00, 0x00);// POWER_SETTING
    EPD_WaitUntilIdle();
    EPD_SendCommand(0x02);// POWER_OFF
}
