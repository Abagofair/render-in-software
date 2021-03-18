#ifndef _E_MATH_H_
#define _E_MATH_H_

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "util.h"

#define PI 3.14159265359

typedef struct {
    float x, y;
} Vector2;

typedef struct {
    float x, y, z;
} Vector3;

typedef struct {
    float m[4][4];
} Matrix4x4;

typedef struct {
    float x, y, z, w;
} Vector4;

Matrix4x4 Identity(void);
Matrix4x4 CreateScale(float sx, float sy, float sz);
Matrix4x4 CreateTranslation(float tx, float ty, float tz);
Matrix4x4 CreateRotationZ(float angle);
Matrix4x4 CreateRotationY(float angle);
Matrix4x4 CreateRotationX(float angle);
Matrix4x4 MatrixMultiplyMatrix(const Matrix4x4* left, const Matrix4x4* right);
Matrix4x4 CreatePerspective(float fov, float aspect, float znear, float zfar);
Vector4 MultiplyVec4Perspective(const Matrix4x4* proj, const Vector4* v);

Vector4 MatrixMultiplyVec4(const Matrix4x4* m, const Vector4* v);
Vector4 Vec3ToVec4(const Vector3* v);

Matrix4x4 LookAt(Vector3 eye, Vector3 target, Vector3 up);

char* Vector2ToString(Vector2 v);

float Vector2Length(Vector2* v);
float Vector3Length(Vector3* v);
Vector2 Vector2Add(Vector2 a, Vector2 b);
Vector3 Vector3Add(Vector3 a, Vector3 b);
Vector2 Vector2Sub(Vector2 a, Vector2 b);
Vector3 Vector3Sub(Vector3 a, Vector3 b);
Vector2 Vector2Scale(Vector2 v, float c);
Vector3 Vector3Scale(Vector3 v, float c);
Vector2 Vector2Div(Vector2 v, float c);
Vector3 Vector3Div(Vector3 v, float c);
float Vector2Dot(Vector2 a, Vector2 b);
float Vector3Dot(Vector3 a, Vector3 b);
Vector3 CrossProduct(Vector3 a, Vector3 b);
void Vector2Norm(Vector2* v);
void Vector3Norm(Vector3* v);
Vector3 RotateX(Vector3 v, float angle);
Vector3 RotateY(Vector3 v, float angle);
Vector3 RotateZ(Vector3 v, float angle);
Vector3 Zero();
Vector3 New(float x, float y, float z);

Vector3 Vector3Clone(Vector3* toClone);

float lerpf(float a, float b, float t);

#endif