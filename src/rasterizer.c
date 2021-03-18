#include "rasterizer.h"

static uint32_t _bufferWidth;
static uint32_t _bufferHeight;
static uint32_t* _pixelBuffer = NULL;
static float* _zBuffer = NULL;

void InitializeRasterizer(uint32_t bufferWidth, uint32_t bufferHeight)
{
    _bufferWidth = bufferWidth;
    _bufferHeight = bufferHeight;

    _pixelBuffer = (uint32_t*)(malloc(sizeof(uint32_t) * bufferHeight * bufferWidth));
    if (!_pixelBuffer)
    {
        fprintf(stderr, "Could not allocate the colorBuffer.\n");
    }

    _zBuffer = (float*)(malloc(sizeof(float) * bufferHeight * bufferWidth));
    if (!_zBuffer)
    {
        fprintf(stderr, "Could not allocate the colorBuffer.\n");
    }
}

void DisposeRasterizer(void)
{
    free(_pixelBuffer);
    free(_zBuffer);
}

static int GetBufferPosition(int x, int y)
{
    return (_bufferWidth * y) + x;
}

static void DrawPixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < _bufferWidth && y >= 0 && y < _bufferHeight)
    {
        _pixelBuffer[GetBufferPosition(x, y)] = color;
    }
}

static void DrawTexel(Vector3* weights,
                float x, float y,
                float u0, float v0,
                float u1, float v1,
                float u2, float v2,
                float recW0, float recW1, float recW2)
{
    float intU = (u0 * recW0) * weights->x + (u1 * recW1) * weights->y + (u2 * recW2) * weights->z;
    float intV = (v0 * recW0) * weights->x + (v1 * recW1) * weights->y + (v2 * recW2) * weights->z;
    
    float interpolatedReciprocalOfW = recW0 * weights->x + recW1 * weights->y + recW2 * weights->z;

    intU /= interpolatedReciprocalOfW;
    intV /= interpolatedReciprocalOfW;

    int texY = abs((int)(intV * textureHeight)) % textureHeight;
    int texX = abs((int)(intU * textureWidth)) % textureWidth;

    int bufferPosition = (textureWidth * texY) + texX;

    interpolatedReciprocalOfW = 1.0 - interpolatedReciprocalOfW;

    int _zBufferPosition = GetBufferPosition(x, y);

    if (_zBufferPosition >= 0 && _zBufferPosition < _bufferWidth * _bufferHeight && interpolatedReciprocalOfW < _zBuffer[_zBufferPosition]) {
        _zBuffer[_zBufferPosition] = interpolatedReciprocalOfW;
        DrawPixel(x, y, meshTexture[bufferPosition]);
    }
}

static void DrawLineDDA(int x0, int y0, int x1, int y1, uint32_t color)
{
    int deltaX = x1 - x0;
    int deltaY = y1 - y0;

    int sideLength = abs(deltaX) >= abs(deltaY) ? abs(deltaX) : abs(deltaY);

    float xInc = deltaX / (float)sideLength;
    float yInc = deltaY / (float)sideLength;

    float currentX = x0;
    float currentY = y0;

    for (int i = 0; i <= sideLength; i++)
    {
        DrawPixel(round(currentX), round(currentY), color);
        currentX += xInc;
        currentY += yInc;
    }
}

static void DrawTriangle(Triangle triangle, uint32_t color)
{
    DrawLineDDA(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, color);
    DrawLineDDA(triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    DrawLineDDA(triangle.points[2].x, triangle.points[2].y, triangle.points[0].x, triangle.points[0].y, color);
}

static void DrawTexturedTriangle(Triangle triangle, uint32_t color, uint32_t* texture)
{
    DrawLineDDA(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, color);
    DrawLineDDA(triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    DrawLineDDA(triangle.points[2].x, triangle.points[2].y, triangle.points[0].x, triangle.points[0].y, color);
}

static void FillTexturedTriangle(Triangle* triangle)
{
    int x0 = triangle->points[0].x;
    int y0 = triangle->points[0].y;
    float z0 = triangle->points[0].z;
    float w0 = triangle->points[0].w;
    float u0 = triangle->texCoords[0].u;
    float v0 = triangle->texCoords[0].v;

    int x1 = triangle->points[1].x;
    int y1 = triangle->points[1].y;
    float z1 = triangle->points[1].z;
    float w1 = triangle->points[1].w;
    float u1 = triangle->texCoords[1].u;
    float v1 = triangle->texCoords[1].v;

    int x2 = triangle->points[2].x;
    int y2 = triangle->points[2].y;
    float z2 = triangle->points[2].z;
    float w2 = triangle->points[2].w;
    float u2 = triangle->texCoords[2].u;
    float v2 = triangle->texCoords[2].v;

    if (y0 > y1) {
        IntSwap(&y0, &y1);
        IntSwap(&x0, &x1);
        FloatSwap(&u0, &u1);
        FloatSwap(&v0, &v1);
        FloatSwap(&z0, &z1);
        FloatSwap(&w0, &w1);
    }
    if (y1 > y2) {
        IntSwap(&y1, &y2);
        IntSwap(&x1, &x2);
        FloatSwap(&u1, &u2);
        FloatSwap(&v1, &v2);
        FloatSwap(&z1, &z2);
        FloatSwap(&w1, &w2);
    }
    if (y0 > y1) {
        IntSwap(&y0, &y1);
        IntSwap(&x0, &x1);
        FloatSwap(&u0, &u1);
        FloatSwap(&v0, &v1);
        FloatSwap(&z0, &z1);
        FloatSwap(&w0, &w1);
    }

    // Flip the V component to account for inverted UV-coordinates (V grows downwards)
    v0 = 1.0 - v0;
    v1 = 1.0 - v1;
    v2 = 1.0 - v2;

    Vector2 a = {.x = x0, .y = y0};
    Vector2 b = {.x = x1, .y = y1};
    Vector2 c = {.x = x2, .y = y2};

    float recW0 = (1 / w0);
    float recW1 = (1 / w1);
    float recW2 = (1 / w2);

    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    int abs_y1y0 = abs(y1 - y0);
    if (abs_y1y0 != 0) 
        inv_slope_1 = (float)(x1 - x0) / abs_y1y0;

    int abs_y2y0 = abs(y2 - y0);
    if (abs_y2y0 != 0) 
        inv_slope_2 = (float)(x2 - x0) / abs_y2y0;

    if (abs_y1y0 != 0)
    {       
        for (int y = y0; y <= y1; y++) 
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                IntSwap(&x_start, &x_end);
            }
            
            for (int x = x_start; x < x_end; ++x)
            {
                Vector2 p = { .x = x, .y = y };
                Vector3 weights = BarycentricWeights(a, b, c, p);
                
                DrawTexel(&weights, p.x, p.y, u0, v0, u1, v1, u2, v2, recW0, recW1, recW2);
            }
        }
    }

    int abs_y2y1 = abs(y2 - y1);
    if (abs_y2y1 != 0) 
    {
        inv_slope_1 = (float)(x2 - x1) / abs_y2y1;
        for (int y = y1; y <= y2; y++) 
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                IntSwap(&x_start, &x_end);
            }
            
            for (int x = x_start; x < x_end; ++x)
            {
                Vector2 p = { .x = x, .y = y };
                Vector3 weights = BarycentricWeights(a, b, c, p);

                DrawTexel(&weights, p.x, p.y, u0, v0, u1, v1, u2, v2, recW0, recW1, recW2);
            }
        }
    }
}

static void ClearColorAndZBuffer(uint32_t color)
{
    for (int y = 0; y < _bufferHeight; ++y)
    {
        for (int x = 0; x < _bufferWidth; ++x)
        {
            DrawPixel(x, y, color);
            _zBuffer[GetBufferPosition(x, y)] = 1.0f;
        }
    }
}

static void DrawGrid(void)
{
    int ratio = _bufferWidth / _bufferHeight;
    for (int y = 0; y < _bufferHeight; ++y)
    {
        for (int x = 0; x < _bufferWidth; ++x)
        {
            if (x % (_bufferWidth / (_bufferWidth / 30)) == 0 || y % (_bufferHeight / (_bufferHeight / 30)) == 0)
                DrawPixel(x, y, 0xFF808080);
            else
                DrawPixel(x, y, 0xFF000000);
        }
    }
}

static void DrawRectangle(int posX, int posY, int width, int height, uint32_t color)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            DrawPixel(x + posX, y + posY, color);
        }
    }
}

void Rasterize(Triangle* triangles, int triangleCount)
{
    //TODO render modes

    ClearColorAndZBuffer(0x6495ED);

    //todo implement pipeline stages

    //app into vertex into fragment into rasterizer into render/display

    for (int i = 0; i < triangleCount; i++)
    {
        FillTexturedTriangle(&triangles[i]);
        /*switch (renderState)
        {
            case WIREFRAME_DOT:
            {
                Triangle t = triangleVec.data[i];
                DrawTriangle(t, t.color);
                DrawRectangle(t.points[0].x, t.points[0].y, 3, 3, 0xFFFF0000);
                DrawRectangle(t.points[1].x, t.points[1].y, 3, 3, 0xFFFF0000);
                DrawRectangle(t.points[2].x, t.points[2].y, 3, 3, 0xFFFF0000);
                break;   
            }
            case WIREFRAME:
            {
                DrawTriangle(t, t.color);
                break;
            }
            case FILLED:
            {
                FillTriangle(t, t.color);
                break;
            }
            case WIREFRAME_FILLED:
            {
                FillTriangle(t, t.color);
                DrawTriangle(t, 0xFF000000);
                break;
            }
            case RENDER_TEXTURED:
            {
                FillTexturedTriangle(t);
                break;
            }
            case RENDER_TEXTURED_WIRE:
            {
                FillTexturedTriangle(t);
                DrawTriangle(t, 0xFF000000);
                break;
            }
            default
        }*/
    }
}

uint32_t* GetPixelBuffer()
{
    return _pixelBuffer;
}

uint32_t GetBufferWidth()
{
    return _bufferWidth;
}

uint32_t GetBufferHeight()
{
    return _bufferHeight;
}

//rewrite bresenham routine
static void PlotLineLow(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int yi = 1;

    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x < x1; x++)
    {
        DrawPixel(x, y, color);
        if (D > 0)
        {
            y = y + yi;
            D = D + (2 * (dy - dx));
        }
        else
        {
            D = D + 2 * dy;
        }
    }
    
}

static void PlotLineHigh(int x0, int y0, int x1, int y1, uint32_t color)
{
    int dx = x1 - x0;
    int dy = y1 - y0;

    int xi = 1;

    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y < y1; y++)
    {
        DrawPixel(x, y, color);
        if (D > 0)
        {
            x = x + xi;
            D = D + (2 * (dx - dy));
        }
        else
        {
            D = D + 2 * dx;
        }
    }
    
}

static void DrawLineBresenham(int x0, int y0, int x1, int y1, uint32_t color)
{
    if (abs(y1 - y0) < abs(x1 - x0))
    {
        if (x0 > x1)
        {
            PlotLineLow(x1, y1, x0, y0, color);
        }
        else
        {
            PlotLineLow(x0, y0, x1, y1, color);
        }
    }
    else
    {
        if (y0 > y1)
        {
            PlotLineLow(x1, y1, x0, y0, color);
        }
        else
        {
            PlotLineLow(x0, y0, x1, y1, color);
        }
    }
}