#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <dirent.h>

#include "e_math.h"
#include "mesh.h"
#include "vec_types.h"
#include "triangle.h"
#include "texture.h"

#include "tga.h"

#define POINTS (9*9*9)

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

TriangleVec triangleVec;

Triangle* trianglesToRender = NULL;

Matrix4x4 perspectiveMatrix;

Vector3 cameraPosition = { 0, 0, -5 };

float fovFactor = 640;

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* colorBufferTexture = NULL;

bool isRunning = false;

int previousFrameTime = 0;

uint32_t* colorBuffer = NULL;

uint16_t windowWidth = 640;
uint16_t windowHeight = 480;

typedef enum RENDER_STATES {
    WIREFRAME_DOT,
    WIREFRAME,
    FILLED,
    WIREFRAME_FILLED,
    RENDER_TEXTURED,
    RENDER_TEXTURED_WIRE
} RENDER_STATES;

typedef struct Light {
    Vector3 direction;
} Light;
Light light;

RENDER_STATES renderState = FILLED;

bool cull = true;

bool InitializeWindow(void)
{
    light.direction = (Vector3){0.0f, 0.0f, 1.0f};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    windowWidth = 640;
    windowHeight = 480;

    window = SDL_CreateWindow(
        "Renderer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        SDL_WINDOW_RESIZABLE
    );

    if (!window)
    {
        fprintf(stderr, "Error creating SDL window.\n");
        return false;
    }

    renderer = SDL_CreateRenderer(
        window,
        -1,
        0
    );

    if (!renderer)
    {
        fprintf(stderr, "Error creating SDL renderer.\n");
        return false;
    }

    SDL_SetWindowFullscreen(window, SDL_WINDOW_RESIZABLE);

    return true;
}

int GetBufferPosition(int x, int y)
{
    return (windowWidth * y) + x;
}

void DrawPixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < windowWidth && y >= 0 && y < windowHeight)
    {
        colorBuffer[GetBufferPosition(x, y)] = color;
    }
}

void DrawLineDDA(int x0, int y0, int x1, int y1, uint32_t color)
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

void DrawTriangle(Triangle triangle, uint32_t color)
{
    DrawLineDDA(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, color);
    DrawLineDDA(triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    DrawLineDDA(triangle.points[2].x, triangle.points[2].y, triangle.points[0].x, triangle.points[0].y, color);
}

void DrawTexturedTriangle(Triangle triangle, uint32_t color, uint32_t* texture)
{
    DrawLineDDA(triangle.points[0].x, triangle.points[0].y, triangle.points[1].x, triangle.points[1].y, color);
    DrawLineDDA(triangle.points[1].x, triangle.points[1].y, triangle.points[2].x, triangle.points[2].y, color);
    DrawLineDDA(triangle.points[2].x, triangle.points[2].y, triangle.points[0].x, triangle.points[0].y, color);
}

uint32_t LightApplyIntensity(uint32_t color, float percentageFactor)
{
    if (percentageFactor < 0)
    {
        percentageFactor = 0;
    }
    if (percentageFactor > 1)
    {
        percentageFactor = 1;
    }

    uint32_t a = (color & 0xFF000000);
    uint32_t r = (color & 0x00FF0000) * percentageFactor;
    uint32_t g = (color & 0x0000FF00) * percentageFactor;
    uint32_t b = (color & 0x000000FF) * percentageFactor;

    return a | (r & 0x00FF0000) | (g & 0x0000FF00) | (b & 0x000000FF);
}

fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color)
{
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    // Start x_start and x_end from the top vertex (x0,y0)
    float x_start = x0;
    float x_end = x0;

    // Loop all the scanlines from top to bottom
    for (int y = y0; y <= y2; y++) {
        DrawLineDDA(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // Find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

    // Start x_start and x_end from the bottom vertex (x2,y2)
    float x_start = x2;
    float x_end = x2;

    // Loop all the scanlines from bottom to top
    for (int y = y2; y >= y0; y--) {
        DrawLineDDA(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}

//flat-bottom and flat-top
void FillTexturedTriangle(Triangle triangle, uint32_t color)
{
    int x0 = triangle.points[0].x;
    int y0 = triangle.points[0].y;
    float z0 = triangle.points[0].z;
    float w0 = triangle.points[0].w;
    float u0 = triangle.texCoords[0].u;
    float v0 = triangle.texCoords[0].v;

    int x1 = triangle.points[1].x;
    int y1 = triangle.points[1].y;
    float z1 = triangle.points[1].z;
    float w1 = triangle.points[1].w;
    float u1 = triangle.texCoords[1].u;
    float v1 = triangle.texCoords[1].v;

    int x2 = triangle.points[2].x;
    int y2 = triangle.points[2].y;
    float z2 = triangle.points[2].z;
    float w2 = triangle.points[2].w;
    float u2 = triangle.texCoords[2].u;
    float v2 = triangle.texCoords[2].v;

    if (y0 > y1) {
        intSwap(&y0, &y1);
        intSwap(&x0, &x1);
        floatSwap(&u0, &u1);
        floatSwap(&v0, &v1);
        floatSwap(&z0, &z1);
        floatSwap(&w0, &w1);
    }
    if (y1 > y2) {
        intSwap(&y1, &y2);
        intSwap(&x1, &x2);
        floatSwap(&u1, &u2);
        floatSwap(&v1, &v2);
        floatSwap(&z1, &z2);
        floatSwap(&w1, &w2);
    }
    if (y0 > y1) {
        intSwap(&y0, &y1);
        intSwap(&x0, &x1);
        floatSwap(&u0, &u1);
        floatSwap(&v0, &v1);
        floatSwap(&z0, &z1);
        floatSwap(&w0, &w1);
    }

    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0) 
        inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0) 
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
    
    if (y1 - y0 != 0)
    {
        // render bottom part of triangle
        for (int y = y0; y <= y1; y++) 
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                intSwap(&x_start, &x_end);
            }
            
            for (int x = x_start; x < x_end; ++x)
            {
                Vector2 a = {.x = x0, .y = y0};
                Vector2 b = {.x = x1, .y = y1};
                Vector2 c = {.x = x2, .y = y2};
                Vector2 p = {.x = x, .y = y};
                Vector3 weights = BarycentricWeights(a, b, c, p);
                
                float intU = (u0 / w0) * weights.x + (u1 / w1) * weights.y + (u2 / w2) * weights.z;
                float intV = (v0 / w0) * weights.x + (v1 / w1) * weights.y + (v2 / w2) * weights.z;
                
                float intRecW = (1 / w0) * weights.x + (1 / w1) * weights.y + (1 / w2) * weights.z;

                intU /= intRecW;
                intV /= intRecW;

                int texY = abs((int)(intV * 64));
                int texX = abs((int)(intU * 64));

                int bufferPosition = (textureWidth * texY) + texX;
                uint32_t texel = meshTexture[bufferPosition];

                DrawPixel(x, y, texel);
            }
        }
    }

    inv_slope_1 = 0;
    inv_slope_2 = 0;

    if (y2 - y1 != 0) 
        inv_slope_1 = (float)(x2 - x1) / abs(y2 - y1);

    if (y2 - y0 != 0) 
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);

    if (y2 - y1 != 0)
    {
        // render bottom part of triangle
        for (int y = y1; y <= y2; y++) 
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                intSwap(&x_start, &x_end);
            }
            
            for (int x = x_start; x < x_end; ++x)
            {
                //compute the barycentric weight for point p
                //using the areas of the triangles created by
                //abp etc..
                Vector2 a = {.x = x0, .y = y0};
                Vector2 b = {.x = x1, .y = y1};
                Vector2 c = {.x = x2, .y = y2};
                Vector2 p = {.x = x, .y = y};
                Vector3 weights = BarycentricWeights(a, b, c, p);

                float intU = (u0 / w0) * weights.x + (u1 / w1) * weights.y + (u2 / w2) * weights.z;
                float intV = (v0 / w0) * weights.x + (v1 / w1) * weights.y + (v2 / w2) * weights.z;
                
                float intRecW = (1 / w0) * weights.x + (1 / w1) * weights.y + (1 / w2) * weights.z;

                intU /= intRecW;
                intV /= intRecW;

                int texY = abs((int)(intV * 64));
                int texX = abs((int)(intU * 64));

                int bufferPosition = (textureWidth * texY) + texX;
                uint32_t texel = meshTexture[bufferPosition];

                DrawPixel(x, y, texel);
            }
        }
    }
}

//flat-bottom and flat-top
void FillTriangle(Triangle triangle, uint32_t color)
{
    int x0 = triangle.points[0].x;
    int y0 = triangle.points[0].y;
    float u0 = triangle.texCoords[0].u;
    float v0 = triangle.texCoords[0].v;

    int x1 = triangle.points[1].x;
    int y1 = triangle.points[1].y;
    float u1 = triangle.texCoords[1].u;
    float v1 = triangle.texCoords[1].v;

    int x2 = triangle.points[2].x;
    int y2 = triangle.points[2].y;
    float u2 = triangle.texCoords[2].u;
    float v2 = triangle.texCoords[2].v;

    if (y0 > y1) {
        intSwap(&y0, &y1);
        intSwap(&x0, &x1);
        floatSwap(&u0, &u1);
        floatSwap(&v0, &v1);
    }
    if (y1 > y2) {
        intSwap(&y1, &y2);
        intSwap(&x1, &x2);
        floatSwap(&u1, &u2);
        floatSwap(&v1, &v2);
    }
    if (y0 > y1) {
        intSwap(&y0, &y1);
        intSwap(&x0, &x1);
        floatSwap(&u0, &u1);
        floatSwap(&v0, &v1);
    }

    if (y1 == y2) {
        // Draw flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    } else if (y0 == y1) {
        // Draw flat-top triangle
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    } else {
        // Calculate the new vertex (Mx,My) using triangle similarity
        int My = y1;
        int Mx = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0;

        // Draw flat-bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);

        // Draw flat-top triangle
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

void ClearColorBuffer(uint32_t color)
{
    for (int y = 0; y < windowHeight; ++y)
    {
        for (int x = 0; x < windowWidth; ++x)
        {
            DrawPixel(x, y, color);
        }
    }
}

void DrawGrid()
{
    int ratio = windowWidth / windowHeight;
    for (int y = 0; y < windowHeight; ++y)
    {
        for (int x = 0; x < windowWidth; ++x)
        {
            if (x % (windowWidth / (windowWidth / 30)) == 0 || y % (windowHeight / (windowHeight / 30)) == 0)
                DrawPixel(x, y, 0xFF808080);
            else
                DrawPixel(x, y, 0xFF000000);
        }
    }
}

void DrawRectangle(int posX, int posY, int width, int height, uint32_t color)
{
    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            DrawPixel(x + posX, y + posY, color);
        }
    }
}

void Setup(void)
{
    //mesh = LoadObj("./assets/monka.obj");
    mesh = LoadObj("./assets/cube.obj");
    //meshTexture = (uint32_t*)REDBRICK_TEXTURE;
    textureWidth = 64;
    textureWidth = 64;
    
    //mesh = LoadObj("./assets/f22.obj");
    
    //mesh.rotation.x += PI;
    //mesh.translation.x = 0.0f;
   // mesh.translation.y = 0.0f;
    //mesh.translation.z = 0.0f;
    //LoadCubeMesh();

    vec_init(&triangleVec);

    colorBuffer = (uint32_t*) malloc(sizeof(uint32_t) * windowWidth * windowHeight);
    if (!colorBuffer)
    {
        fprintf(stderr, "Could not allocate the colorBuffer.\n");
    }

    colorBufferTexture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        windowWidth,
        windowHeight
    );

    perspectiveMatrix = CreatePerspective(PI / 2.0f, windowWidth / windowHeight, 0.1f, 1000.0f);
}

bool IsKeyPressed(const SDL_Event* event, int keyCode)
{
    return event->key.keysym.sym == keyCode;
}

void Input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
        case SDL_QUIT:
            isRunning = false;
            break;
        case SDL_KEYDOWN:
            {
                if (IsKeyPressed(&event, SDLK_ESCAPE))
                {
                    isRunning = false;
                }
                else if (IsKeyPressed(&event, SDLK_1))
                {
                    renderState = WIREFRAME_DOT;
                }
                else if (IsKeyPressed(&event, SDLK_2))
                {
                    renderState = WIREFRAME;
                }
                else if (IsKeyPressed(&event, SDLK_3))
                {
                    renderState = FILLED;
                }
                else if (IsKeyPressed(&event, SDLK_4))
                {
                    renderState = WIREFRAME_FILLED;
                }
                else if (IsKeyPressed(&event, SDLK_5))
                {
                    renderState = RENDER_TEXTURED;
                }
                else if (IsKeyPressed(&event, SDLK_6))
                {
                    renderState = RENDER_TEXTURED_WIRE;
                }
                else if (IsKeyPressed(&event, SDLK_c))
                {
                    cull = true;
                }
                else if (IsKeyPressed(&event, SDLK_d))
                {
                    cull = false;
                }

                if (IsKeyPressed(&event, SDLK_w))
                {
                    mesh.rotation.x += 0.10;
                }
                else if (IsKeyPressed(&event, SDLK_s))
                {
                    mesh.rotation.x -= 0.10;
                }
                else if (IsKeyPressed(&event, SDLK_a))
                {
                    mesh.rotation.y += 0.10;
                }
                else if (IsKeyPressed(&event, SDLK_d))
                {
                    mesh.rotation.y -= 0.10;
                }

                break;
            }
    }
}

Vector4 Project(Vector4 point)
{
    Vector4 vec4Point = MultiplyVec4Perspective(&perspectiveMatrix, &point);
    return vec4Point;
}

bool BackfaceCull(const Vector3 triangles[])
{
    Vector3 ba = Vector3Sub(triangles[1], triangles[0]);
    Vector3Norm(&ba);
    Vector3 ca = Vector3Sub(triangles[2], triangles[0]);
    Vector3Norm(&ca);

    Vector3 faceNorm = CrossProduct(ba, ca);
    Vector3Norm(&faceNorm);

    Vector3 vec3a = triangles[0];

    Vector3 aToCamera = Vector3Sub(cameraPosition, vec3a);

    float cameraRayDotFaceNormal = Vector3Dot(faceNorm, aToCamera);

    return cameraRayDotFaceNormal < 0.0f ? true : false;
}

void Update(void)
{
    /*int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - previousFrameTime);

    if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(timeToWait);
    }*/

    vec_clear(&triangleVec);

//mesh.rotation.x += 0.01;
    //mesh.rotation.y += 0.01;
  //  mesh.rotation.z += 0.01;

   // mesh.scale.x += 0.001;
   // mesh.scale.y += 0.001;
   // mesh.scale.z += 0.001;

   //mesh.translation.x += 0.01;
    mesh.translation.z = 5.0f;

    Matrix4x4 scaleMatrix = CreateScale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    Matrix4x4 transMatrix = CreateTranslation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    Matrix4x4 rotMatrixX = CreateRotationX(mesh.rotation.x);
    Matrix4x4 rotMatrixY = CreateRotationY(mesh.rotation.y);
    Matrix4x4 rotMatrixZ = CreateRotationZ(mesh.rotation.z);

    Matrix4x4 world = Identity();
    //world = MatrixMultiplyMatrix(&world, &scaleMatrix);
    world = MatrixMultiplyMatrix(&world, &rotMatrixX);
    world = MatrixMultiplyMatrix(&world, &rotMatrixY);
    world = MatrixMultiplyMatrix(&world, &rotMatrixZ);
    world = MatrixMultiplyMatrix(&world, &transMatrix);

    for (int i = 0; i < mesh.faces.length; i++)
    {
        Face meshFace = mesh.faces.data[i];

        Vector3 faceVertices[FACE_SIZE];
        faceVertices[0] = mesh.vertices.data[meshFace.a - 1];
        faceVertices[1] = mesh.vertices.data[meshFace.b - 1];
        faceVertices[2] = mesh.vertices.data[meshFace.c - 1];

        Vector3 transformedVertices[FACE_SIZE];
        for (int j = 0; j < FACE_SIZE; j++)
        {
            Vector4 transformedVertex = Vec3ToVec4(&faceVertices[j]);
            
            transformedVertex = MatrixMultiplyVec4(&world, &transformedVertex);

            transformedVertices[j] = (Vector3){
                .x = transformedVertex.x,
                .y = transformedVertex.y,
                .z = transformedVertex.z
            };
        }

        if (cull && BackfaceCull(transformedVertices))
        {
            continue;
        }

        Vector3 ba = Vector3Sub(transformedVertices[1], transformedVertices[0]);
        Vector3Norm(&ba);
        Vector3 ca = Vector3Sub(transformedVertices[2], transformedVertices[0]);
        Vector3Norm(&ca);
        Vector3 faceNorm = CrossProduct(ba, ca);
        Vector3Norm(&faceNorm);

        float dot = -Vector3Dot(faceNorm, light.direction);

        Triangle projectedTriangle;
        float avgZ = 0.0f;
        for (int j = 0; j < FACE_SIZE; j++)
        {
            Vector3 transformedVertex = transformedVertices[j];
            avgZ += transformedVertex.z;
            
            Vector4 projectedPoint = Project(Vec3ToVec4(&transformedVertex));

            projectedTriangle.points[j] = (Vector4){
                .x = projectedPoint.x,
                .y = projectedPoint.y * -1,
                .z = projectedPoint.z,
                .w = projectedPoint.w
            };

            projectedTriangle.points[j].x *= (windowWidth / 2.0);
            projectedTriangle.points[j].y *= (windowHeight / 2.0);

            projectedTriangle.points[j].x += (windowWidth / 2.0);
            projectedTriangle.points[j].y += (windowHeight / 2.0);
        }

        projectedTriangle.texCoords[0] = (Texture2D){
            .u = meshFace.a_uv.u, 
            .v = meshFace.a_uv.v
        };
        projectedTriangle.texCoords[1] = (Texture2D){
            .u = meshFace.b_uv.u, 
            .v = meshFace.b_uv.v
        };
        projectedTriangle.texCoords[2] = (Texture2D){
            .u = meshFace.c_uv.u, 
            .v = meshFace.c_uv.v
        };

        projectedTriangle.color = LightApplyIntensity(0xFFFF0000, dot);
        projectedTriangle.avgDepth = avgZ / 3;

        vec_push(&triangleVec, projectedTriangle);
    }
}

int Compare(const void * elem1, const void * elem2)
{
    Triangle a = *((Triangle*)elem1);
    Triangle b = *((Triangle*)elem2);
    if (a.avgDepth > b.avgDepth) return -1;
    if (a.avgDepth < b.avgDepth) return 1;
    return 0;
}

void Render(void)
{
    ClearColorBuffer(0x6495ED);

    //DrawGrid();

    qsort(triangleVec.data, triangleVec.length, sizeof(Triangle), Compare);

    for (int i = 0; i < triangleVec.length; i++)
    {
        Triangle t = triangleVec.data[i];
        switch (renderState)
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
                FillTexturedTriangle(t, t.color);
                break;
            }
            case RENDER_TEXTURED_WIRE:
            {
                FillTexturedTriangle(t, t.color);
                DrawTriangle(t, 0xFF000000);
                break;
            }
            default:
                break;
        }
    }

    SDL_UpdateTexture(
        colorBufferTexture,
        NULL,
        colorBuffer,
        (int)(windowWidth * sizeof(uint32_t))
    );

    vec_clear(&triangleVec);

    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);

    SDL_RenderPresent(renderer);
}

void PrintString(char* str)
{
    printf(str);
    free(str);
}

int main(int argc, char** args)
{
    isRunning = InitializeWindow();

    TGA_t* tga = ReadUncompressedRGB("./assets/redbrick.tga");
    meshTexture = tga->colorBuffer;

    Setup();

    while (isRunning)
    {
        Input();
        Update();
        Render();
    }

    free(colorBuffer);
    Dispose(tga);

    SDL_Quit();

    return 0;
}
