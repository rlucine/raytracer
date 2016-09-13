/**********************************************************//**
 * @file shape.h
 * @brief Header file for 3D shape definitons
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _SHAPE_H_
#define _SHAPE_H_

// This project
#include "rgb.h"
#include "vector.h"

/**********************************************************//**
 * @typedef SHAPE_KEY
 * @brief Integer representing every kind of shape
 **************************************************************/
typedef enum {
    SHAPE_NONE = 0,
    SHAPE_SPHERE,
    SHAPE_ELLIPSOID,
} SHAPE_KEY;

/**********************************************************//**
 * @typedef SHAPE_DATA
 * @brief Union of shape-specific data
 **************************************************************/
typedef union {
    double radius;      ///< Radius of a sphere
    VECTOR dimension;   ///< Dimensions of an ellipsoid
} SHAPE_DATA;

/**********************************************************//**
 * @typedef SHAPE
 * @brief Polymorphic struct for storing shape data
 **************************************************************/
typedef struct {
    SHAPE_KEY shape;    ///< Shape key for which field of the union is initialized
    RGB color;          ///< Color of this shape
    POINT center;       ///< Location of this shape
    SHAPE_DATA data;    ///< Polymorphic shape data
} SHAPE;

/*============================================================*/
#endif // _SHAPE_H_
