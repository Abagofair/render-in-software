#include "util.h"

void FloatSwap(float* a, float* b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

void IntSwap(int* a, int* b) 
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}