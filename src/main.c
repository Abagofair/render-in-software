#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "e_math.h"
#include "mesh.h"

#include "render.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define POINTS (9*9*9)

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

bool isRunning = false;

int previousFrameTime = 0;

typedef struct Light {
    Vector3 direction;
} Light;
Light light;

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

    uint32_t a = (color & 0x000000FF);
    uint32_t r = (color & 0xFF000000) * percentageFactor;
    uint32_t g = (color & 0x00FF0000) * percentageFactor;
    uint32_t b = (color & 0x0000FF00) * percentageFactor;

    return (r & 0xFF000000) | (g & 0x00FF0000) | (b & 0x0000FF00) | a;
}

void Setup(void)
{
    mesh = LoadObj("./assets/drone.obj");
    
    textureHeight = 512;
    textureWidth = 512;
    int channels = 4;

    meshTexture = (uint32_t*)stbi_load("./assets/drone.png", &textureHeight, &textureWidth, &channels, 0);
}

bool IsKeyPressed(const SDL_Event* event, int keyCode)
{
    return event->key.keysym.sym == keyCode;
}

float currentRotY = 0.0f;

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

                if (IsKeyPressed(&event, SDLK_w))
                {
                    camera.velocity.z = -0.5;
                }
                else if (IsKeyPressed(&event, SDLK_s))
                {
                    camera.velocity.z = 0.5;
                }
                else if (IsKeyPressed(&event, SDLK_a))
                {
                    camera.velocity.x = 0.1;
                }
                else if (IsKeyPressed(&event, SDLK_d))
                {
                    camera.velocity.x = -0.1;
                }

                if (IsKeyPressed(&event, SDLK_LEFT))
                {
                    camera.rotation.y = 0.05f;
                    camera.direction = RotateY(camera.direction, camera.rotation.y);
                }

                return;
            }
        case SDL_KEYUP:
        {
            if (IsKeyPressed(&event, SDLK_w))
            {
                camera.velocity.z = 0.0;
            }
            else if (IsKeyPressed(&event, SDLK_s))
            {
                camera.velocity.z = 0.0;
            }
            else if (IsKeyPressed(&event, SDLK_a))
            {
                camera.velocity.x = 0.0;
            }
            else if (IsKeyPressed(&event, SDLK_d))
            {
                camera.velocity.x = 0.0;
            }

            return;
        }
    }
}

void PrintString(char* str)
{
    printf(str);
    free(str);
}

int main(int argc, char** args)
{
    isRunning = InitializeRenderer(PI / 3.0, 2, 10000);

    Setup();

    Mesh meshes[1];
    meshes[0] = mesh;

    while (isRunning)
    {
        Input();
        //Update();
        Render(meshes, 1);
    }

    SDL_Quit();

    DisposeRasterizer();

    return 0;
}
