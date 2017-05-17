/**********************************************************//**
 * @file geometry.h
 * @brief Header file for basic vector geometry.
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _GEOMETRY_H_
#define _GEOMETRY_H_

// This project
#include "vector.h"

/**********************************************************//**
 * @struct LINE
 * @brief Struct for an infinite line emanating from the origin
 * in the given direction. Direction need not be normalized.
 **************************************************************/
typedef struct {
    VECTOR origin;      ///< Origin of the line
    VECTOR direction;   ///< Direction of the line
} LINE;

/**********************************************************//**
 * @struct PLANE
 * @brief Struct for a flat plane. The point origin is on
 * the plane and the vectors u and v form the basis of the
 * plane space. u and v need not be normalized.
 **************************************************************/
typedef struct {
    VECTOR origin;  ///< Upper left corner of the plane
    VECTOR u;       ///< Step of width in the plane
    VECTOR v;       ///< Step of height in the plane
} PLANE;

/*============================================================*/
#endif // _GEOMETRY_H_