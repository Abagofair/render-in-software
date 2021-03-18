#ifndef _MESH_H_
#define _MESH_H_

#include <stdio.h>

#include "e_math.h"
#include "triangle.h"
#include "vec_types.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) //6 cube faces, 2 triangles pr face

extern Vector3 cubeVertices[N_CUBE_VERTICES];
extern Face cubeFaces[N_CUBE_FACES];

typedef struct 
{
    Vector3Vec vertices;
    FaceVec faces;
    Texture2DVec uvCoords;
    Vector3 rotation;
    Vector3 scale;
    Vector3 translation;
} Mesh;

extern Mesh mesh;

void LoadCubeMesh(void);
Mesh LoadObj(const char* fileName);

#endif