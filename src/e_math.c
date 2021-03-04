#include "e_math.h"

Matrix4x4 Identity(void)
{
    Matrix4x4 m = {{
        {1, 0, 0, 0},
        {0, 1, 0, 0},
        {0, 0, 1, 0},
        {0, 0, 0, 1}
    }};

    return m;
}

Matrix4x4 CreateScale(float sx, float sy, float sz)
{
    Matrix4x4 m = Identity();
    m.m[0][0] = sx;
    m.m[1][1] = sy;
    m.m[2][2] = sz;
    return m;
}

Matrix4x4 CreateTranslation(float tx, float ty, float tz)
{
    Matrix4x4 m = Identity();
    m.m[0][3] = tx;
    m.m[1][3] = ty;
    m.m[2][3] = tz;
    return m;
}

Matrix4x4 CreateRotationZ(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    Matrix4x4 m = Identity();
    m.m[0][0] = c;
    m.m[0][1] = -s;
    m.m[1][0] = s;
    m.m[1][1] = c;
    return m;
}

Matrix4x4 CreateRotationY(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    Matrix4x4 m = Identity();
    m.m[0][0] = c;
    m.m[0][2] = s;
    m.m[2][0] = -s;
    m.m[2][2] = c;
    return m;
}

Matrix4x4 CreateRotationX(float angle)
{
    float c = cos(angle);
    float s = sin(angle);

    Matrix4x4 m = Identity();
    m.m[1][1] = c;
    m.m[1][2] = -s;
    m.m[2][1] = s;
    m.m[2][2] = c;
    return m;
}

Vector4 MatrixMultiplyVec4(const Matrix4x4* m, const Vector4* v)
{
    Vector4 result = {
        .x = m->m[0][0] * v->x + m->m[0][1] * v->y + m->m[0][2] * v->z + m->m[0][3] * v->w,
        .y = m->m[1][0] * v->x + m->m[1][1] * v->y + m->m[1][2] * v->z + m->m[1][3] * v->w,
        .z = m->m[2][0] * v->x + m->m[2][1] * v->y + m->m[2][2] * v->z + m->m[2][3] * v->w,
        .w = m->m[3][0] * v->x + m->m[3][1] * v->y + m->m[3][2] * v->z + m->m[3][3] * v->w
    };
    return result;
}

Matrix4x4 MatrixMultiplyMatrix(const Matrix4x4* left, const Matrix4x4* right)
{
    Matrix4x4 result = {
        .m[0][0] = right->m[0][0] * left->m[0][0] + right->m[0][1] * left->m[1][0] + right->m[0][2] * left->m[2][0] + right->m[0][3] * left->m[3][0],
        .m[0][1] = right->m[0][0] * left->m[0][1] + right->m[0][1] * left->m[1][1] + right->m[0][2] * left->m[2][1] + right->m[0][3] * left->m[3][1],
        .m[0][2] = right->m[0][0] * left->m[0][2] + right->m[0][1] * left->m[1][2] + right->m[0][2] * left->m[2][2] + right->m[0][3] * left->m[3][2],
        .m[0][3] = right->m[0][0] * left->m[0][3] + right->m[0][1] * left->m[1][3] + right->m[0][2] * left->m[2][3] + right->m[0][3] * left->m[3][3],

        .m[1][0] = right->m[1][0] * left->m[0][0] + right->m[1][1] * left->m[1][0] + right->m[1][2] * left->m[2][0] + right->m[1][3] * left->m[3][0],
        .m[1][1] = right->m[1][0] * left->m[0][1] + right->m[1][1] * left->m[1][1] + right->m[1][2] * left->m[2][1] + right->m[1][3] * left->m[3][1],
        .m[1][2] = right->m[1][0] * left->m[0][2] + right->m[1][1] * left->m[1][2] + right->m[1][2] * left->m[2][2] + right->m[1][3] * left->m[3][2],
        .m[1][3] = right->m[1][0] * left->m[0][3] + right->m[1][1] * left->m[1][3] + right->m[1][2] * left->m[2][3] + right->m[1][3] * left->m[3][3],

        .m[2][0] = right->m[2][0] * left->m[0][0] + right->m[2][1] * left->m[1][0] + right->m[2][2] * left->m[2][0] + right->m[2][3] * left->m[3][0],
        .m[2][1] = right->m[2][0] * left->m[0][1] + right->m[2][1] * left->m[1][1] + right->m[2][2] * left->m[2][1] + right->m[2][3] * left->m[3][1],
        .m[2][2] = right->m[2][0] * left->m[0][2] + right->m[2][1] * left->m[1][2] + right->m[2][2] * left->m[2][2] + right->m[2][3] * left->m[3][2],
        .m[2][3] = right->m[2][0] * left->m[0][3] + right->m[2][1] * left->m[1][3] + right->m[2][2] * left->m[2][3] + right->m[2][3] * left->m[3][3],

        .m[3][0] = right->m[3][0] * left->m[0][0] + right->m[3][1] * left->m[1][0] + right->m[3][2] * left->m[2][0] + right->m[3][3] * left->m[3][0],
        .m[3][1] = right->m[3][0] * left->m[0][1] + right->m[3][1] * left->m[1][1] + right->m[3][2] * left->m[2][1] + right->m[3][3] * left->m[3][1],
        .m[3][2] = right->m[3][0] * left->m[0][2] + right->m[3][1] * left->m[1][2] + right->m[3][2] * left->m[2][2] + right->m[3][3] * left->m[3][2],
        .m[3][3] = right->m[3][0] * left->m[0][3] + right->m[3][1] * left->m[1][3] + right->m[3][2] * left->m[2][3] + right->m[3][3] * left->m[3][3]
    };
    return result;
}

Matrix4x4 CreatePerspective(float fov, float aspect, float znear, float zfar)
{
    Matrix4x4 m = Identity();
    m.m[0][0] = aspect * (1 / tan(fov / 2));
    m.m[1][1] = 1 / tan(fov/2);
    m.m[2][2] = zfar / (zfar - znear);
    m.m[2][3] = (-zfar * znear) / (zfar - znear);
    m.m[3][2] = 1.0f;
    return m;
}

Vector4 MultiplyVec4Perspective(const Matrix4x4* proj, const Vector4* v)
{
    Vector4 result = MatrixMultiplyVec4(proj, v);

    if (result.w != 0.0f)
    {
        result.x /= result.w;
        result.y /= result.w;
        result.z /= result.w;
    }

    return result;
}

Vector4 Vec3ToVec4(const Vector3* v)
{
    return (Vector4) {
        .x = v->x,
        .y = v->y,
        .z = v->z,
        .w = 1.0f
    };
}

char* Vector2ToString(Vector2 v)
{
    char* str = (char*)malloc(sizeof(char*) * 255);
    sprintf(str, "(%f, %f)\n",
        v.x,
        v.y);
    return str;
}

float Vector2Length(Vector2* v)
{
    return sqrt(v->x * v->x + v->y * v->y);
}

float Vector3Length(Vector3* v)
{
    return sqrt(v->x * v->x + v->y * v->y + v->z * v->z);
}

Vector2 Vector2Add(Vector2 a, Vector2 b)
{
    return (Vector2) {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
}

Vector3 Vector3Add(Vector3 a, Vector3 b)
{
    return (Vector3) {
        .x = a.x + b.x,
        .y = a.y + b.y,
        .z = a.z + b.z
    };
}

Vector2 Vector2Sub(Vector2 a, Vector2 b)
{
    return (Vector2) {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
}

Vector3 Vector3Sub(Vector3 a, Vector3 b)
{
    return (Vector3) {
        .x = a.x - b.x,
        .y = a.y - b.y,
        .z = a.z - b.z
    };
}

Vector2 Vector2Scale(Vector2 v, float c)
{
    return (Vector2) {
        .x = v.x * c,
        .y = v.y * c
    };
}

Vector3 Vector3Scale(Vector3 v, float c)
{
    return (Vector3) {
        .x = v.x * c,
        .y = v.y * c,
        .z = v.z * c
    };
}

Vector2 Vector2Div(Vector2 v, float c)
{
    return (Vector2) {
        .x = v.x / c,
        .y = v.y / c
    };
}

Vector3 Vector3Div(Vector3 v, float c)
{
    return (Vector3) {
        .x = v.x / c,
        .y = v.y / c,
        .z = v.z / c
    };
}

float Vector2Dot(Vector2 a, Vector2 b)
{
    return a.x * b.x + a.y * b.y;
}

float Vector3Dot(Vector3 a, Vector3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vector3 CrossProduct(Vector3 a, Vector3 b)
{
    return (Vector3) {
        .x = a.y * b.z - a.z * b.y,
        .y = a.z * b.x - a.x * b.z,
        .z = a.x * b.y - a.y * b.x
    };
}

void Vector2Norm(Vector2* v)
{
    float length = Vector2Length(v);
    v->x /= length;
    v->y /= length;
}

void Vector3Norm(Vector3* v)
{
    float length = Vector3Length(v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

Vector3 RotateX(Vector3 v, float angle)
{
    Vector3 rotatedVector = {
        .x = v.x,
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle),
    };

    return rotatedVector;
}

Vector3 RotateY(Vector3 v, float angle)
{
    Vector3 rotatedVector = {
        .x = v.x * cos(angle) - v.z * sin(angle),
        .y = v.y,
        .z = v.x * sin(angle) + v.z * cos(angle),
    };

    return rotatedVector;
}

Vector3 RotateZ(Vector3 v, float angle)
{
    Vector3 rotatedVector = {
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z
    };

    return rotatedVector;
}

Vector3 Zero()
{
    return (Vector3){ 0, 0 ,0 };
}