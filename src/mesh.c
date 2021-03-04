#include "mesh.h"

Mesh mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0},
    .scale = {1.0, 1.0, 1.0},
    .translation = {0, 0, 0}
};

Vector3 cubeVertices[N_CUBE_VERTICES] = {
    {-1, -1, -1},
    {-1, 1, -1},
    {1, 1, -1},
    {1, -1, -1},
    {1, 1, 1},
    {1, -1, 1},
    {-1, 1, 1},
    {-1, -1, 1},
};

Face cubeFaces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 1, .b = 3, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // right
    { .a = 4, .b = 3, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 4, .b = 5, .c = 6, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // back
    { .a = 6, .b = 5, .c = 7, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 7, .c = 8, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // left
    { .a = 8, .b = 7, .c = 2, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 8, .b = 2, .c = 1, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // top
    { .a = 2, .b = 7, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 2, .b = 5, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // bottom
    { .a = 6, .b = 8, .c = 1, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 1, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF }
};

void LoadCubeMesh(void)
{
    vec_init(&mesh.vertices);
    vec_init(&mesh.faces);

    for (int i = 0; i < N_CUBE_VERTICES; i++)
    {
        Vector3 cubeVertex = cubeVertices[i];
        vec_push(&mesh.vertices, cubeVertex);
    }
    
    for (int i = 0; i < N_CUBE_FACES; i++)
    {
        Face cubeFace = cubeFaces[i];
        vec_push(&mesh.faces, cubeFace);
    }
}

typedef struct Obj {
    FaceVec faces;
    Vector3Vec vecs;
    vec_void_t uvIndex;
    vec_void_t vertexIndex;
    Texture2DVec vts;
} Obj_t;

//use the popular library i cant remember what's called - everyone uses it
Mesh LoadObj(const char* fileName)
{
    FILE* objFile;

    objFile = fopen(fileName, "r");
    if (objFile == NULL)
    {
        perror("Error opening file!");
    }

    char str[255];

    Mesh objMesh = {
        .vertices = {0, 0, 0},
        .faces = {0, 0, 0},
        .rotation = {0, 0, 0}
    };

    Obj_t obj;
    vec_init(&obj.faces);
    vec_init(&obj.vecs);
    vec_init(&obj.uvIndex);
    vec_init(&obj.vertexIndex);
    vec_init(&obj.vts);

    vec_init(&objMesh.vertices);
    vec_init(&objMesh.faces);

//shits broken
    while (fgets(str, 1024, objFile))
    {
        if (strncmp(str, "v ", 2) == 0)
        {
            Vector3 vec;
            sscanf(str, "%*s %f %f %f", &vec.x, &vec.y, &vec.z);
            vec_push(&objMesh.vertices, vec);
        }
        else if (strncmp(str, "f ", 2) == 0)
        {
            Face face;
            int uvIndex[3];

            sscanf(str, "%*s %i/%i/%*i %i/%i/%*i %i/%i/%*i", 
                &face.a, &face.b, &face.c,
                &uvIndex, &uvIndex, &uvIndex);

            vec_push(&objMesh.faces, face);
            vec_push(&obj.uvIndex, uvIndex);
        }
        else if (strncmp(str, "vt ", 2) == 0)
        {
            Texture2D tex;
            sscanf(str, "%*s %d %d", &tex.u, &tex.v);
            vec_push(&obj.vts, tex);
        }
    }

    for (int i = 0; i < mesh.faces.length; i++)
    {
        Face* face = &mesh.faces.data[i];
        int* indexes = obj.uvIndex.data[i];
        face->a_uv  = obj.vts.data[indexes[0]];
        face->b_uv  = obj.vts.data[indexes[1]];
        face->c_uv  = obj.vts.data[indexes[2]];
    }
    
    fclose(objFile);

    return objMesh;
}