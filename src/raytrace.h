/**********************************************************//**
 * @file raytrace.h
 * @brief Header file for raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

// This project
#include "image.h"  // IMAGE
#include "scene.h"  // SCENE
#include "color.h"  // COLOR
#include "shape.h"  // COLLISION

/**********************************************************//**
 * @def VIEW_DISTANCE
 * @brief Distance of viewing plane from eye
 **************************************************************/
#define VIEW_DISTANCE 1.0

/**********************************************************//**
 * @def SHADOW_THRESHOLD
 * @brief Considered completely dark if under this value
 * because 0.003 < (1 / 255).
 **************************************************************/
#define SHADOW_THRESHOLD 0.003

/**********************************************************//**
 * @def COLLISION_THRESHOLD
 * @brief Only consider collisions greater than this distance
 * for shadow casting.
 **************************************************************/
#define COLLISION_THRESHOLD 0.001

/**********************************************************//**
 * @def INITIAL_REFRACTION
 * @brief The index of refraction of the material
 * containing the eye
 **************************************************************/
#define INITIAL_REFRACTION 1.0

/**********************************************************//**
 * @def RECURSION_DEPTH
 * @brief The maximum recursion depth for recursive ray
 * tracing.
 **************************************************************/
#define RECURSION_DEPTH 5

/**********************************************************//**
 * @brief Shoot one ray into the scene and get a COLLISION.
 * @param closest: Output location of the closest collision.
 * @param ray: The ray to shoot.
 * @param scene: The scene the objects are in
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int raytrace_Cast(COLLISION *closest, const LINE *ray, const SCENE *scene);

/**********************************************************//**
 * @brief Determine the color at the given collision
 * @param color: The color at the collision
 * @param collision: The collision to deal with
 * @param scene: The scene the collision is in
 * @param depth: The maximum recursion depth.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int raytrace_Shade(COLOR *color, const COLLISION *collision, const SCENE *scene, int depth);

/**********************************************************//**
 * @brief Render the SCENE using a raytracing algorithm
 * @param image: The image to write to
 * @param scene: The scene to render
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int raytrace_Render(IMAGE *image, const SCENE *scene);

/*============================================================*/
#endif // _RAYTRACE_H_
