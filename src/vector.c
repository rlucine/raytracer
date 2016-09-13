/**********************************************************//**
 * @file vector.c
 * @brief Header file for VECTOR operations
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <float.h>      // DBL_EPSILON
#include <math.h>       // sqrt, acos

// This project
#include "vector.h"

/*============================================================*
 * Zero Checking
 *============================================================*/
static int double_IsZero(double what) {
    return what < DBL_EPSILON;
}

/*============================================================*
 * Dot product
 *============================================================*/
double vector_Dot(const VECTOR *a, const VECTOR *b) {
    return a->x*b->x + a->y*b->y + a->z*b->z;
}

/*============================================================*
 * Magnitude
 *============================================================*/
double vector_Magnitude(const VECTOR *vector) {
    return sqrt(vector->x*vector->x + vector->y*vector->y + vector->z*vector->z);
}

/*============================================================*
 * Angle
 *============================================================*/
double vector_Angle(const VECTOR *a, const VECTOR *b) {
    return acos(vector_Dot(a, b) / (vector_Magnitude(a) * vector_Magnitude(b)));
}

/*============================================================*
 * Checking
 *============================================================*/
int vector_IsZero(const VECTOR *vector) {
    return double_IsZero(vector->x) && double_IsZero(vector->y) && double_IsZero(vector->z);
}

int vector_IsParalell(const VECTOR *a, const VECTOR *b) {
    return double_IsZero(vector_Angle(a, b));
}

int vector_IsOrthogonal(const VECTOR *a, const VECTOR *b) {
    return double_IsZero(vector_Dot(a, b));
}

int vector_IsEqual(const VECTOR *a, const VECTOR *b) {
    return double_IsZero(a->x - b->x) && double_IsZero(a->y - b->y) && double_IsZero(a->z - b->z);
}

int vector_IsUnit(const VECTOR *a) {
    return double_IsZero(vector_Magnitude(a) - 1.0);
}

/*============================================================*
 * Math operations
 *============================================================*/
void vector_Add(VECTOR *output, const VECTOR *a, const VECTOR *b) {
    output->x = a->x + b->x;
    output->y = a->y + b->y;
    output->z = a->z + b->z;
}

void vector_Subtract(VECTOR *output, const VECTOR *a, const VECTOR *b) {
    output->x = a->x - b->x;
    output->y = a->y - b->y;
    output->z = a->z - b->z;
}

void vector_Negate(VECTOR *output, const VECTOR *a) {
    output->x = -a->x;
    output->y = -a->y;
    output->z = -a->z;
}

void vector_Cross(VECTOR *output, const VECTOR *a, const VECTOR *b) {
    // Buffer these before assigning to output because output
    // might be a or b
    int out_x, out_y, out_z;
    out_x = a->y*b->z - a->z*b->y;
    out_y = a->x*b->z - a->z*b->x;
    out_z = a->x*b->y - a->y*b->x;
    
    output->x = out_x;
    output->y = out_y;
    output->z = out_z;
}

void vector_Unit(VECTOR *output, const VECTOR *a) {
    double magnitude = vector_Magnitude(a);
    if (!double_IsZero(magnitude)) {
        output->x = a->x / magnitude;
        output->y = a->y / magnitude;
        output->z = a->z / magnitude;
    } else {
        output->x = 0.0;
        output->y = 0.0;
        output->z = 0.0;
    }
}

void vector_Multiply(VECTOR *output, const VECTOR *a, double scale) {
    output->x = a->x * scale;
    output->y = a->y * scale;
    output->z = a->z * scale;
}

/*============================================================*/