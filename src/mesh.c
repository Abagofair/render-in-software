#include "mesh.h"

Mesh mesh = {
    .vertices = NULL,
    .faces = NULL,
    .uvCoords = NULL,
    .rotation = {0, 0, 0},
    .scale = {1.0, 1.0, 1.0},
    .translation = {0, 0, 0}
};

/*Vector3 cubeVertices[N_CUBE_VERTICES] = {
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
};*/

/*void LoadCubeMesh(void)
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
}*/

//use the popular library i cant remember what's called - everyone uses it
Mesh LoadObj(const char* fileName)
{
    FILE* objFile;

    objFile = fopen(fileName, "r");
    if (objFile == NULL)
    {
        perror("Error opening file!");
    }

    char str[1024];

    Mesh objMesh;

    vec_init(&objMesh.vertices);
    vec_init(&objMesh.faces);
    vec_init(&objMesh.uvCoords);

//shits broken
    while (fgets(str, 1024, objFile))
    {
        if (strncmp(str, "v ", 2) == 0)
        {
            Vector3 vec;
            sscanf(str, "v %f %f %f", &vec.x, &vec.y, &vec.z);
            vec_push(&objMesh.vertices, vec);
        }
        else if (strncmp(str, "f ", 2) == 0)
        {
            Face face;
            sscanf(str, "f %d/%d/%*d %d/%d/%*d %d/%d/%*d",
                &face.a, &face.a_uv, 
                &face.b, &face.b_uv, 
                &face.c, &face.c_uv);

            vec_push(&objMesh.faces, face);
        }
        else if (strncmp(str, "vt ", 3) == 0)
        {
            Texture2D coords;
            sscanf(str, "vt %f %f", &coords.u, &coords.v);
            vec_push(&objMesh.uvCoords, coords);
        }
    }
    
    fclose(objFile);

    return objMesh;
}