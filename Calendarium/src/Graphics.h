#pragma once

class Graphics
{
public:
	static int GetWidth();
	static int GetHeight();
	static void Init();
	static void Clear();
	static void DrawRectangle(int x, int y, int width, int height);
	static void DrawRectangleFromImage(int x, int y, int width, int height, int scale, const unsigned char* image, int imageWidth, int imageHeight, int imagePosX, int imagePosY);
	static void DrawText(int x, int y, const char* text, int scale = 1);
	static void Flush();
};
