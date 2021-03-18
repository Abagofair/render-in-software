#ifndef _CLIPPING_H_
#define _CLIPPING_H_

#include "e_math.h"
#include "texture.h"

enum {
    LEFT_FRUSTUM_PLANE,
    RIGHT_FRUSTUM_PLANE,
    TOP_FRUSTUM_PLANE,
    BOTTOM_FRUSTUM_PLANE,
    NEAR_FRUSTUM_PLANE,
    FAR_FRUSTUM_PLANE
};

typedef struct
{
    Vector3 point;
    Vector3 normal;
} Plane_t;

typedef struct
{
    Vector3 vertices[10];
    Texture2D texCoords[10];
    int numVertices;
} Polygon_t;


void InitFrustumPlanes(float fovX, float fovY, float zNear, float zFar);
Polygon_t CreatePolygonFromTriangle(
    Vector3* a,
    Vector3* b,
    Vector3* c,
	Texture2D* aUv,
	Texture2D* bUv,
	Texture2D* cUv);
void ClipPolygonAgainstPlane(Polygon_t* polygon, int plane);
void ClipPolygon(Polygon_t* polygon);

#endif