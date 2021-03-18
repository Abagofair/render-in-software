#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "e_math.h"

typedef struct Camera {
    Vector3 position;
    Vector3 direction;
    Vector3 velocity;
    Vector3 rotation;
} Camera_t;

#endif