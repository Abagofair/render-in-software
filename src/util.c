#include "util.h"

void floatSwap(float* a, float* b)
{
    float temp = *a;
    *a = *b;
    *b = temp;
}

void intSwap(int* a, int* b) 
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}