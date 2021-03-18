#include "clipping.h"

#define NUM_PLANES 6
Plane_t frustum_planes[NUM_PLANES];

void InitFrustumPlanes(float fovX, float fovY, float zNear, float zFar)
{
    float cos_half_fov_y = cos(fovY / 2);
	float sin_half_fov_y = sin(fovY / 2);
	float cos_half_fov_x = cos(fovX / 2);
	float sin_half_fov_x = sin(fovX / 2);

	frustum_planes[LEFT_FRUSTUM_PLANE].point = Zero();
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fov_x;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = Zero();
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fov_x;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fov_x;

	frustum_planes[TOP_FRUSTUM_PLANE].point = Zero();
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fov_y;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = Zero();
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fov_y;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fov_y;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = New(0, 0, zNear);
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

	frustum_planes[FAR_FRUSTUM_PLANE].point = New(0, 0, zFar);
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

Polygon_t CreatePolygonFromTriangle(
    Vector3* a,
    Vector3* b,
    Vector3* c,
	Texture2D* aUv,
	Texture2D* bUv,
	Texture2D* cUv)
{
	Polygon_t polygon = {
		.vertices = {*a, *b, *c},
		.numVertices = 3,
		.texCoords = {*aUv, *bUv, *cUv }
	};

	return polygon;
}

//JoyKennethClipping - UCDavis
void ClipPolygonAgainstPlane(Polygon_t* polygon, int plane)
{
	Vector3 planePoint = frustum_planes[plane].point;
	Vector3 planeNormal = frustum_planes[plane].normal;

	Vector3 insideVertices[10];
	Texture2D insideTexCoords[10];
	int verticesInside = 0;

	Vector3* currentVertex = &polygon->vertices[0];
	Texture2D* currentTexCoord = &polygon->texCoords[0];

	Vector3* previousVertex = &polygon->vertices[polygon->numVertices - 1];
	Texture2D* previousTexCoord = &polygon->texCoords[polygon->numVertices - 1];

	float currentDot = 0;
	float previousDot = Vector3Dot(Vector3Sub(*previousVertex, planePoint), planeNormal);

	while (currentVertex != &polygon->vertices[polygon->numVertices])
	{
		currentDot = Vector3Dot(Vector3Sub(*currentVertex, planePoint), planeNormal);

		if (currentDot * previousDot < 0)
		{
			float t = previousDot / (previousDot - currentDot);

			Vector3 intersectionPoint = {
				.x = lerpf(previousVertex->x, currentVertex->x, t),
				.y = lerpf(previousVertex->y, currentVertex->y, t),
				.z = lerpf(previousVertex->z, currentVertex->z, t)
			};

			Texture2D interpolatedTexCoord = {
				.u = lerpf(previousTexCoord->u, currentTexCoord->u, t),
				.v = lerpf(previousTexCoord->v, currentTexCoord->v, t)
			};

			insideVertices[verticesInside] = Vector3Clone(&intersectionPoint);
			insideTexCoords[verticesInside] = (Texture2D) {
				.u = interpolatedTexCoord.u,
				.v = interpolatedTexCoord.v
			};
			++verticesInside;
		}

		if (currentDot > 0)
		{
			insideVertices[verticesInside] = Vector3Clone(currentVertex);
			insideTexCoords[verticesInside] = (Texture2D) {
				.u = currentTexCoord->u,
				.v = currentTexCoord->v
			};
			++verticesInside;
		}

		previousDot = currentDot;
		previousTexCoord = currentTexCoord;
		previousVertex = currentVertex;
		++currentTexCoord;
		++currentVertex;
	}

	for (int i = 0; i < verticesInside; i++)
	{
		polygon->vertices[i] = Vector3Clone(&insideVertices[i]);
		polygon->texCoords[i] = (Texture2D) {
			.u = insideTexCoords[i].u,
			.v = insideTexCoords[i].v
		};
	}
	
	polygon->numVertices = verticesInside;
}

void ClipPolygon(Polygon_t* polygon)
{
	ClipPolygonAgainstPlane(polygon, LEFT_FRUSTUM_PLANE);
	ClipPolygonAgainstPlane(polygon, RIGHT_FRUSTUM_PLANE);
	ClipPolygonAgainstPlane(polygon, TOP_FRUSTUM_PLANE);
	ClipPolygonAgainstPlane(polygon, BOTTOM_FRUSTUM_PLANE);
	ClipPolygonAgainstPlane(polygon, NEAR_FRUSTUM_PLANE);
	ClipPolygonAgainstPlane(polygon, FAR_FRUSTUM_PLANE);
}