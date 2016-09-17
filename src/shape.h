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
 * @typedef SHAPE_TYPE
 * @brief Integer representing every kind of shape
 **************************************************************/
typedef enum {
    SHAPE_NONE = 0,     ///< The shape is not initialized
    SHAPE_SPHERE,       ///< The shape is a sphere
    SHAPE_ELLIPSOID,    ///< The shape is an ellipsoid
} SHAPE_TYPE;

/**********************************************************//**
 * @typedef SPHERE
 * @brief Struct for storing sphere data
 **************************************************************/
typedef struct {
    POINT center;   ///< The center of the sphere
    double radius;  ///< The radius of the sphere
} SPHERE;

/**********************************************************//**
 * @typedef ELLIPSOID
 * @brief Struct for storing ellipsoid data
 **************************************************************/
typedef struct {
    POINT center;       ///< The center of the ellipsoid
    VECTOR dimension;   ///< The dimensions of the ellipsoid
} ELLIPSOID;

/**********************************************************//**
 * @typedef MATERIAL
 * @brief Struct for storing object material data
 **************************************************************/
typedef struct {
    RGB color;      ///< The color of the object
} MATERIAL;

/**********************************************************//**
 * @typedef SHAPE
 * @brief Polymorphic struct for storing shape data
 * SHAPE is not responsible for allocating or freeing the data
 * field, as this field may also be stack-allocated.
 **************************************************************/
typedef struct {
    SHAPE_TYPE shape;       ///< Key representing the type of geometry
    void *data;             ///< Pointer to the shape geometry
    MATERIAL material;      ///< The material of the object
} SHAPE;

/**********************************************************//**
 * @typedef COLLISION_TYPE
 * @brief Integer representing what type of collision occurred
 **************************************************************/
typedef enum {
    COLLISION_NONE = 0,     ///< No collision occurred
    COLLISION_INSIDE,       ///< The point is inside the shape
    COLLISION_SURFACE,      ///< The point is on the surface
} COLLISION_TYPE;

/**********************************************************//**
 * @typedef COLLISION
 * @brief Data about the collision with a shape
 **************************************************************/
typedef struct {
    COLLISION_TYPE how; ///< How the collision occurred
    VECTOR where;       ///< Point of collision on the surface
    double distance;    ///< How far away the point is
} COLLISION;

/*============================================================*/
#endif // _SHAPE_H_
