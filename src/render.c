#include "render.h"

bool InitializeRenderer(float verticalFov, uint32_t rasterBufferRatio, uint32_t triangles)
{
    //light.direction = (Vector3){0.0f, 0.0f, 1.0f};

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        fprintf(stderr, "Error initializing SDL.\n");
        return false;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);

    uint32_t bufferWidth = displayMode.w / rasterBufferRatio;
    uint32_t bufferHeight = displayMode.h / rasterBufferRatio;

    InitializeRasterizer(bufferWidth, bufferHeight);

    InitializePerspectiveMatrix(verticalFov, bufferWidth, bufferHeight);

    vec_init(&triangleVec);
    vec_reserve(&triangleVec, triangles);

    window = SDL_CreateWindow(
        "Renderer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        displayMode.w,
        displayMode.h,
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

    colorBufferTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, bufferWidth, bufferHeight);

    SDL_SetWindowFullscreen(window, SDL_WINDOW_RESIZABLE);

    camera.position = (Vector3) {
        0.0, 1.0, 25.5,
    };
    camera.direction = (Vector3){ 0, 0, -1.0};

    return true;
}

static void InitializePerspectiveMatrix(float verticalFov, uint32_t bufferWidth, uint32_t bufferHeight)
{
    float aspectY = (float)bufferHeight / (float)bufferWidth;
    float aspectX = (float)bufferWidth / (float)bufferHeight;
    float fovY = verticalFov; // the same as 180/3, or 60deg
    float fovX = atan(tan(fovY / 2) * aspectX) * 2;
    float zNear = 0.01;
    float zFar = 1000.0;

    perspectiveMatrix = CreatePerspective(fovY, aspectX, zNear, zFar);
    InitFrustumPlanes(fovX, fovY, zNear, zFar);
}

static bool BackfaceCull(const Vector3* triangles)
{
    Vector3 ba = Vector3Sub(triangles[1], triangles[0]);
    Vector3Norm(&ba);
    Vector3 ca = Vector3Sub(triangles[2], triangles[0]);
    Vector3Norm(&ca);

    Vector3 faceNorm = CrossProduct(ba, ca);
    Vector3Norm(&faceNorm);

    Vector3 vec3a = triangles[0];

    Vector3 aToCamera = Vector3Sub((Vector3){0, 0, 0}, vec3a);

    float cameraRayDotFaceNormal = Vector3Dot(faceNorm, aToCamera);

    return cameraRayDotFaceNormal < 0.0f ? true : false;
}

Vector4 Project(Vector4 point)
{
    Vector4 vec4Point = MultiplyVec4Perspective(&perspectiveMatrix, &point);
    return vec4Point;
}

void Render(Mesh* meshes, int meshCount)
{
    Matrix4x4 world = Identity();

    camera.position = Vector3Add(camera.position, camera.velocity);
    Vector3 target = Vector3Add(camera.position, camera.direction);
    Matrix4x4 viewMatrix = LookAt(
        camera.position,
        target,
        (Vector3){0.0, 1.0, 0.0}
    );

    for (int i = 0; i < meshCount; i++)
    {
        Mesh* mesh = &meshes[i];
        RenderMesh(mesh, &world, &viewMatrix);
    }
    
    Rasterize(triangleVec.data, triangleVec.length);

    SDL_UpdateTexture(colorBufferTexture, NULL, GetPixelBuffer(), (int)(GetBufferWidth() * sizeof(uint32_t)));
    SDL_RenderCopy(renderer, colorBufferTexture, NULL, NULL);
    SDL_RenderPresent(renderer);

    vec_clear(&triangleVec);
}

static void RenderMesh(const Mesh* mesh, const Matrix4x4* world, const Matrix4x4* view)
{
    Vector3 faceVertices[FACE_SIZE];
    Vector3 transformedVertices[FACE_SIZE];
    uint32_t bufferWidth = GetBufferWidth();
    uint32_t bufferHeight = GetBufferHeight();

    for (int i = 0; i < mesh->faces.length; i++)
    {
        Face meshFace = mesh->faces.data[i];

        faceVertices[0] = mesh->vertices.data[meshFace.a - 1];
        faceVertices[1] = mesh->vertices.data[meshFace.b - 1];
        faceVertices[2] = mesh->vertices.data[meshFace.c - 1];

        for (int j = 0; j < FACE_SIZE; j++)
        {
            Vector4 transformedVertex = Vec3ToVec4(&faceVertices[j]);
            
            transformedVertex = MatrixMultiplyVec4(world, &transformedVertex);
            transformedVertex = MatrixMultiplyVec4(view, &transformedVertex);

            transformedVertices[j] = (Vector3){
                .x = transformedVertex.x,
                .y = transformedVertex.y,
                .z = transformedVertex.z
            };
        }

        if (BackfaceCull(transformedVertices)) continue;

        Polygon_t polygon = CreatePolygonFromTriangle(
            &transformedVertices[0],
            &transformedVertices[1],
            &transformedVertices[2],
            &mesh->uvCoords.data[meshFace.a_uv - 1],
            &mesh->uvCoords.data[meshFace.b_uv - 1],
            &mesh->uvCoords.data[meshFace.c_uv - 1]
        );

        ClipPolygon(&polygon);

        Triangle clippedTriangles[10];
        int numberOfClippedTriangles = 0;

        Vector3 q1 = polygon.vertices[0];
        for (int i = 0; i < (polygon.numVertices - 2); i++)
        {
            Vector3 q2 = polygon.vertices[i + 1];
            Vector3 q3 = polygon.vertices[i + 2];

            clippedTriangles[numberOfClippedTriangles++] = (Triangle) {
                .points = {
                    (Vector4) { .x = q1.x, .y = q1.y, .z = q1.z},
                    (Vector4) { .x = q2.x, .y = q2.y, .z = q2.z},
                    (Vector4) { .x = q3.x, .y = q3.y, .z = q3.z}
                },
                .texCoords = {
                    (Texture2D) { .u = polygon.texCoords[0].u, .v = polygon.texCoords[0].v },
                    (Texture2D) { .u = polygon.texCoords[1].u, .v = polygon.texCoords[1].v },
                    (Texture2D) { .u = polygon.texCoords[2].u, .v = polygon.texCoords[2].v }
                }
            };
        }

        for (int i = 0; i < numberOfClippedTriangles; i++)
        {
            Triangle triangle = clippedTriangles[i];
            Vector3 trannies[3];

            trannies[0] = (Vector3) { 
                .x = triangle.points[0].x, 
                .y = triangle.points[0].y, 
                .z = triangle.points[0].z
            };
            
            trannies[1] = (Vector3) { 
                .x = triangle.points[1].x, 
                .y = triangle.points[1].y, 
                .z = triangle.points[1].z
            };
            
            trannies[2] = (Vector3) { 
                .x = triangle.points[2].x, 
                .y = triangle.points[2].y, 
                .z = triangle.points[2].z
            };

            //light correction
            /*Vector3 ba = Vector3Sub(trannies[1], trannies[0]);
            Vector3Norm(&ba);
            Vector3 ca = Vector3Sub(trannies[2], trannies[0]);
            Vector3Norm(&ca);
            Vector3 faceNorm = CrossProduct(ba, ca);
            Vector3Norm(&faceNorm);

            float dot = -Vector3Dot(faceNorm, light.direction);*/

            for (int j = 0; j < FACE_SIZE; j++)
            {
                Vector3 transformedVertex = trannies[j];
                
                Vector4 projectedPoint = Project(Vec3ToVec4(&transformedVertex));

                triangle.points[j] = (Vector4){
                    .x = projectedPoint.x,
                    .y = projectedPoint.y * -1,
                    .z = projectedPoint.z,
                    .w = projectedPoint.w
                };

                triangle.points[j].x *= (bufferWidth / 2.0);
                triangle.points[j].y *= (bufferHeight / 2.0);

                triangle.points[j].x += (bufferWidth / 2.0);
                triangle.points[j].y += (bufferHeight / 2.0);
            }

            //triangle.color = LightApplyIntensity(0xFFFF0000, dot);

            vec_push(&triangleVec, triangle);
        }
    }   
}