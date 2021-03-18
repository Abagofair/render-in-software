#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#include <math.h>
#include <stdint.h>

#include "triangle.h"
#include "e_math.h"

void InitializeRasterizer(uint32_t bufferWidth, uint32_t bufferHeight);
void DisposeRasterizer(void);

uint32_t GetBufferWidth();
uint32_t GetBufferHeight();


static int GetBufferPosition(int x, int y);
static void DrawPixel(int x, int y, uint32_t color);
static void DrawTexel(Vector3* weights,
                 float x, float y,
                 float u0, float v0,
                 float u1, float v1,
                 float u2, float v2,
                 float recW0, float recW1, float recW2);
 
static void PlotLineLow(int x0, int y0, int x1, int y1, uint32_t color);
static void PlotLineHigh(int x0, int y0, int x1, int y1, uint32_t color);
static void DrawLineBresenham(int x0, int y0, int x1, int y1, uint32_t color);
static void DrawLineDDA(int x0, int y0, int x1, int y1, uint32_t color);
static void DrawTriangle(Triangle triangle, uint32_t color);
static void DrawTexturedTriangle(Triangle triangle, uint32_t color, uint32_t* texture);
static void FillTexturedTriangle(Triangle* triangle);
static void ClearColorAndZBuffer(uint32_t color);
static void DrawGrid(void);
static void DrawRectangle(int posX, int posY, int width, int height, uint32_t color);
void Rasterize(Triangle* triangles, int triangleCount);
uint32_t* GetPixelBuffer();

#endif