/**********************************************************//**
 * @file shape.h
 * @brief Header file for 3D shape definitons
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _SHAPE_H_
#define _SHAPE_H_

// This project
#include "macro.h"
#include "image.h"
#include "vector.h"

/**********************************************************//**
 * @enum SHAPE_TYPE
 * @brief Integer representing every kind of shape
 **************************************************************/
typedef enum {
    SHAPE_NONE = 0,     ///< The shape is not initialized
    SHAPE_SPHERE,       ///< The shape is a sphere
    SHAPE_ELLIPSOID,    ///< The shape is an ellipsoid
} SHAPE_TYPE;

/**********************************************************//**
 * @struct SPHERE
 * @brief Struct for storing sphere data
 **************************************************************/
typedef struct {
    POINT center;   ///< The center of the sphere
    double radius;  ///< The radius of the sphere
} SPHERE;

/**********************************************************//**
 * @struct ELLIPSOID
 * @brief Struct for storing ellipsoid data
 **************************************************************/
typedef struct {
    POINT center;       ///< The center of the ellipsoid
    VECTOR dimension;   ///< The dimensions of the ellipsoid
} ELLIPSOID;

/**********************************************************//**
 * @struct MATERIAL
 * @brief Struct for storing object material data
 **************************************************************/
typedef struct {
    RGB color;      ///< The color of the object
} MATERIAL;

/**********************************************************//**
 * @struct SHAPE
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
 * @enum COLLISION_TYPE
 * @brief Integer representing what type of collision occurred
 **************************************************************/
typedef enum {
    COLLISION_NONE = 0,     ///< No collision occurred
    COLLISION_INSIDE,       ///< The point is inside the shape
    COLLISION_SURFACE,      ///< The point is on the surface
} COLLISION_TYPE;

/**********************************************************//**
 * @struct COLLISION
 * @brief Data about the collision with a shape
 **************************************************************/
typedef struct {
    COLLISION_TYPE how; ///< How the collision occurred
    VECTOR where;       ///< Point of collision on the surface
    double distance;    ///< How far away the point is
} COLLISION;

/**********************************************************//**
 * @brief Construct a shape of the given properties
 * @param shape: The shape to construct. It must be destroyed
 * with shape_Destroy if this function is successful.
 * @param type: The shape type
 * @param data: Pointer to a block of data corresponding to
 * the shape type. SPHERE* or ELLIPSOID* are valid.
 * @param material: Pointer to the material properties
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int shape_Create(SHAPE *shape, SHAPE_TYPE type, const void *data, const MATERIAL *material);

/**********************************************************//**
 * @brief Destroy a shape struct
 * @param shape: The shape to destroy. It cannot be accessed
 * after calling this function.
 **************************************************************/
extern void shape_Destroy(SHAPE *shape);

/**********************************************************//**
 * @brief Get the material properties of the SHAPE
 * @param shape: The shape to read
 * @return Pointer to a MATERIAL struct
 **************************************************************/
extern const MATERIAL *shape_GetMaterial(const SHAPE *shape);

/**********************************************************//**
 * @brief Get the SPHERE data embedded in the shape.
 * @param shape: The shape to read
 * @return Pointer to a SPHERE struct on success, or NULL if
 * the shape is not a sphere.
 **************************************************************/
extern const SPHERE *shape_GetSphere(const SHAPE *shape);

/**********************************************************//**
 * @brief Get the ELLIPSOID data embedded in the shape.
 * @param shape: The shape to read
 * @return Pointer to a ELLIPSOID struct on success, or NULL if
 * the shape is not an ellipsoid.
 **************************************************************/
extern const ELLIPSOID *shape_GetEllipsoid(const SHAPE *shape);

/**********************************************************//**
 * @brief Determine the shape's geometry.
 * @param shape: The shape to read
 * @return The SHAPE_TYPE representing the geometry, or
 * SHAPE_NONE on failure.
 **************************************************************/
extern SHAPE_TYPE shape_GetGeometry(const SHAPE *shape);

/**********************************************************//**
 * @brief Get the center of the sphere.
 * @param sphere: Pointer to a SPHERE struct
 * @return Pointer to the POINT of the center of the sphere
 **************************************************************/
extern const POINT *sphere_GetCenter(const SPHERE *sphere);

/**********************************************************//**
 * @brief Get the radius of the sphere.
 * @param sphere: Pointer to a SPHERE struct
 * @return The positive value of the radius
 **************************************************************/
extern double sphere_GetRadius(const SPHERE *sphere);

/**********************************************************//**
 * @brief Get the center of the ellipsoid.
 * @param ellipsoid: Pointer to an ELLIPSOID struct
 * @return Pointer to the POINT of the center of the ellipsoid
 **************************************************************/
extern const POINT *ellipsoid_GetCenter(const ELLIPSOID *ellipsoid);

/**********************************************************//**
 * @brief Get the dimensions of the ellipsoid as a vector.
 * @param ellipsoid: Pointer to an ELLIPSOID struct
 * @return Pointer to the VECTOR containing the dimensions
 **************************************************************/
extern const VECTOR *ellipsoid_GetDimension(const ELLIPSOID *ellipsoid);

/**********************************************************//**
 * @brief Collide the line with the sphere and generate
 * collision data.
 * @param sphere: The sphere to collide with
 * @param ray: The line to intersect with the sphere
 * @param result: Output location for collision data
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int sphere_Collide(const SPHERE *sphere, const LINE *ray, COLLISION *result);

/**********************************************************//**
 * @brief Collide the line with the ellipsoid and generate
 * collision data.
 * @param ellipsoid: The ellipsoid to collide with
 * @param ray: The line to intersect with the ellipsoid
 * @param result: Output location for collision data
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ellipsoid_Collide(const ELLIPSOID *ellipsoid, const LINE *ray, COLLISION *result);

/**********************************************************//**
 * @brief Generalized collision function for all SHAPE_TYPE
 * @param shape: The shape to collide with
 * @param ray: The line to intersect with the shape
 * @param result: Output location for collision data
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int shape_Collide(const SHAPE *shape, const LINE *ray, COLLISION *result);

/*============================================================*/
#endif // _SHAPE_H_
