#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <stdint.h>

#include "e_math.h"

typedef struct Texture2D {
    float u;
    float v;
} Texture2D;

extern int textureWidth;
extern int textureHeight;
extern uint32_t* meshTexture;
extern const uint8_t REDBRICK_TEXTURE[];

#endif