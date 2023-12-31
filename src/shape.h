/**********************************************************//**
 * @file shape.h
 * @brief Header file for 3D shape geometry and storage
 * @author Rena Shinomiya
 **************************************************************/

#ifndef _SHAPE_H_
#define _SHAPE_H_

// Standard library
#include <stdbool.h>    // bool

// This project
#include "image.h"      // TEXTURE, TEXCOORD
#include "vector.h"     // VECTOR
#include "mesh.h"       // FACE
#include "geometry.h"   // PLANE, LINE

/**********************************************************//**
 * @enum SHAPE_TYPE
 * @brief Keys representing every kind of SHAPE geometry
 **************************************************************/
typedef enum {
    SHAPE_NONE = 0,     ///< The shape is not initialized
    SHAPE_SPHERE,       ///< The shape is a sphere
    SHAPE_ELLIPSOID,    ///< The shape is an ellipsoid
    SHAPE_PLANE,        ///< The shape is a flat plane
    SHAPE_FACE,         ///< The shape is a triangle
} SHAPE_TYPE;

/**********************************************************//**
 * @struct SPHERE
 * @brief Struct for storing sphere data
 **************************************************************/
typedef struct {
    VECTOR center;   ///< The center of the sphere
    float radius;  ///< The radius of the sphere
} SPHERE;

/**********************************************************//**
 * @struct ELLIPSOID
 * @brief Struct for storing ellipsoid data
 **************************************************************/
typedef struct {
    VECTOR center;       ///< The center of the ellipsoid
    VECTOR dimension;   ///< The dimensions of the ellipsoid
} ELLIPSOID;

/**********************************************************//**
 * @struct MATERIAL
 * @brief Struct for storing object material data
 **************************************************************/
typedef struct {
    COLOR color;            ///< The diffuse color of the object
    COLOR highlight;        ///< The specular color of the object
    float ambient;         ///< The ambient reflectivity of the object
    float diffuse;         ///< The diffuse reflectivity of the object
    float specular;        ///< The specular reflectivity of the object
    int exponent;           ///< The size of the specular highlight
    const TEXTURE *texture; ///< The texture to apply to the shape
    float opacity;         ///< The object's opacity
    float refraction;      ///< The object's index of refraction
} MATERIAL;

/**********************************************************//**
 * @struct SHAPE
 * @brief Generalized storage of shape data. The SHAPE_TYPE
 * must correspond exactly to the target of the data pointer.
 * SHAPE_SPHERE corresponds to a SPHERE.
 * SHAPE_ELLIPSOID corresponds to an ELLIPSOID.
 * No further shape types are supported.
 **************************************************************/
typedef struct {
    SHAPE_TYPE shape;           ///< Key representing the type of geometry
    void *data;                 ///< Pointer to the shape geometry
    const MATERIAL *material;   ///< The material of the object
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
    COLLISION_TYPE how;         ///< How the collision occurred
    VECTOR where;               ///< Point of collision on the surface
    float distance;            ///< How far away the point is
    const MATERIAL *material;   ///< The material that was collided with
    VECTOR incident;            ///< Vector pointing towards the origin
    VECTOR normal;              ///< Normal vector at collision site
    const TEXTURE *texture;     ///< Texture collided with or NULL
    TEXCOORD texcoord;          ///< Texture coordinate at the collision site
} COLLISION;

/**********************************************************//**
 * @brief Construct a sphere shape of the given properties.
 * @param shape: The shape to construct. It must be destroyed
 * with shape_Destroy if this function is successful. If this
 * function is not successful, it will clean up any memory
 * allocations and shape_Destroy will not work.
 * @param sphere: Pointer to SPHERE definition
 * @param material: Pointer to the material properties. The
 * data pointed to may be destroyed after calling this function.
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool shape_CreateSphere(SHAPE *shape, const SPHERE *sphere, const MATERIAL *material);

/**********************************************************//**
 * @brief Construct an ellipsoid shape of the given properties.
 * @param shape: The shape to construct. It must be destroyed
 * with shape_Destroy if this function is successful. If this
 * function is not successful, it will clean up any memory
 * allocations and shape_Destroy will not work.
 * @param ellipsoid: Pointer to ELLIPSOID definition
 * @param material: Pointer to the material properties. The
 * data pointed to may be destroyed after calling this function.
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool shape_CreateEllipsoid(SHAPE *shape, const ELLIPSOID *ellipsoid, const MATERIAL *material);

/**********************************************************//**
 * @brief Construct a plane with the given properties.
 * @param shape: The shape to construct. It must be destroyed
 * with shape_Destroy if this function is successful. If this
 * function is not successful, it will clean up any memory
 * allocations and shape_Destroy will not work.
 * @param plane: Pointer to a PLANE definition.
 * @param material: Pointer to the material properties. The
 * data pointed to may be destroyed after calling this function.
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool shape_CreatePlane(SHAPE *shape, const PLANE *plane, const MATERIAL *material);

/**********************************************************//**
 * @brief Construct a triangular face for a polygon.
 * @param shape: The shape to construct. It must be destroyed
 * with shape_Destroy if this function is successful. If this
 * function is not successful, it will clean up any memory
 * allocations and shape_Destroy will not work.
 * @param face: Pointer to a FACE definition.
 * @param material: Pointer to the material properties. The
 * data pointed to may be destroyed after calling this function.
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool shape_CreateFace(SHAPE *shape, const FACE *face, const MATERIAL *material);

/**********************************************************//**
 * @brief Destroy a shape struct that has been initialized by
 * shape_Create. You cannot destroy a shape that has not
 * been initialized, and you cannot destroy a shape twice.
 * @param shape: The shape to destroy. It cannot be accessed
 * after calling this function.
 **************************************************************/
extern void shape_Destroy(SHAPE *shape);

/**********************************************************//**
 * @brief Get the material properties of the SHAPE
 * @param shape: The shape to read
 * @return Pointer to a MATERIAL struct
 **************************************************************/
static inline const MATERIAL *shape_GetMaterial(const SHAPE *shape) {
    return shape->material;
}

/**********************************************************//**
 * @brief Set the material properties of the shape
 * @param shape: The shape to set
 * @param material: Pointer to the material struct
 **************************************************************/
static inline void shape_SetMaterial(SHAPE *shape, const MATERIAL *material) {
    shape->material = material;
}

/**********************************************************//**
 * @brief Generalized collision function for all SHAPE_TYPE
 * @param shape: The shape to collide with
 * @param ray: The line to intersect with the shape
 * @param result: Output location for collision data. If the
 * collision type is COLLISION_NONE, no other data fields will
 * be valid. Otherwise, all data fields will be written to.
 * @return Whether the collision detection succeeded.
 **************************************************************/
extern bool shape_Collide(const SHAPE *shape, const LINE *ray, COLLISION *result);

/**********************************************************//**
 * @brief Get the true color of the shape at the given
 * collision.
 * @param collision: A collision with a shape.
 * @param color: Output location for the color.
 * @return Whether color was extracted successfully.
 **************************************************************/
extern bool shape_GetColorAt(const COLLISION *collision, COLOR *color);

/*============================================================*/
#endif // _SHAPE_H_
