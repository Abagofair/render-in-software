#ifndef _TRIANGLE_H_
#define _TRIANGLE_H_

#include "e_math.h"
#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define FACE_SIZE 3

typedef struct
{
    int a;
    int b;
    int c;
    Texture2D a_uv;
    Texture2D b_uv;
    Texture2D c_uv;
    uint32_t color;
} Face;

typedef struct
{
    Vector4 points[3];
    Texture2D texCoords[3];
    uint32_t color;
    float avgDepth;
} Triangle;

Triangle SortTriangle(Triangle triangle);
Vector2 TriangleMidpoint(Triangle orderedTriangle);
char* TriangleToString(Triangle triangle);
Vector3 BarycentricWeights(Vector2 a, Vector2 b, Vector2 c, Vector2 p);

#endif