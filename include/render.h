#ifndef _RENDER_H_
#define _RENDER_H_

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "rasterizer.h"
#include "e_math.h"
#include "clipping.h"
#include "vec.h"
#include "mesh.h"
#include "camera.h"
#include "vec_types.h"

static SDL_Window* window = NULL;
static SDL_Renderer* renderer = NULL;
static SDL_Texture* colorBufferTexture = NULL;

static Matrix4x4 perspectiveMatrix;
static Matrix4x4 viewMatrix;
static Camera_t camera;
static TriangleVec triangleVec;

bool InitializeRenderer(float verticalFov, uint32_t rasterBufferRatio, uint32_t triangles);
static void InitializePerspectiveMatrix(float verticalFov, uint32_t bufferWidth, uint32_t bufferHeight);
static bool BackfaceCull(const Vector3* triangles);
Vector4 Project(Vector4 point);
void Render(Mesh* meshes, int meshCount);
static void RenderMesh(const Mesh* mesh, const Matrix4x4* world, const Matrix4x4* view);

#endif