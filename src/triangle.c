#include "triangle.h"

//Returns an ordered triangle where y0 < y1 < y2
Triangle SortTriangle(Triangle triangle)
{
    float x0 = triangle.points[0].x;
    float y0 = triangle.points[0].y;
    
    float x1 = triangle.points[1].x;
    float y1 = triangle.points[1].y;

    float x2 = triangle.points[2].x;
    float y2 = triangle.points[2].y;

    if (y0 > y1)
    {
        FloatSwap(&y0, &y1);
        FloatSwap(&x0, &x1);
    }

    if (y1 > y2)
    {
        FloatSwap(&y1, &y2);
        FloatSwap(&x1, &x2);
    }

    if (y0 > y1)
    {
        FloatSwap(&y0, &y1);
        FloatSwap(&x0, &x1);
    }

    return (Triangle) {
        .points[0] = { x0, y0 },
        .points[1] = { x1, y1 },
        .points[2] = { x2, y2 }
    };
}

//Expects an ordered triangle where y0 < y1 < y2
Vector2 TriangleMidpoint(Triangle orderedTriangle)
{
    float x0 = orderedTriangle.points[0].x;
    float y0 = orderedTriangle.points[0].y;

    float x1 = orderedTriangle.points[1].x;
    float y1 = orderedTriangle.points[1].y;

    float x2 = orderedTriangle.points[2].x;
    float y2 = orderedTriangle.points[2].y;
    
    float midpointX = (((x2 - x0) * (y1 - y0)) / (y2 - y0)) + x0;

    return (Vector2) {
        .x = midpointX,
        .y = y1
    };
}

char* TriangleToString(Triangle triangle)
{
    char* str = (char*)malloc(sizeof(char*) * 255);
    sprintf(str, "x0y0 (%f, %f) | x1y1 (%f, %f) | x2y2 (%f, %f)\n",
        triangle.points[0].x,
        triangle.points[0].y,
        triangle.points[1].x,
        triangle.points[1].y,
        triangle.points[2].x,
        triangle.points[2].y);
    return str;
}

Vector3 BarycentricWeights(Vector2 a, Vector2 b, Vector2 c, Vector2 p)
{
    Vector2 ab = Vector2Sub(b, a);
    Vector2 bc = Vector2Sub(c, b);
    Vector2 ac = Vector2Sub(c, a);
    Vector2 ap = Vector2Sub(p, a);
    Vector2 bp = Vector2Sub(p, b);

    // Calcualte the area of the full triangle ABC using cross product (area of parallelogram)
    float area_triangle_abc = (ab.x * ac.y - ab.y * ac.x);

    // Weight alpha is the area of subtriangle BCP divided by the area of the full triangle ABC
    float alpha = (bc.x * bp.y - bp.x * bc.y) / area_triangle_abc;

    // Weight beta is the area of subtriangle ACP divided by the area of the full triangle ABC
    float beta = (ap.x * ac.y - ac.x * ap.y) / area_triangle_abc;

    // Weight gamma is easily found since barycentric cooordinates always add up to 1
    float gamma = 1 - alpha - beta;

    return (Vector3) {
        .x = alpha, 
        .y = beta, 
        .z = gamma
    };
}
