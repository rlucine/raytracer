/**********************************************************//**
 * @file raytrace.h
 * @brief Header file for raytracer implementation
 * @author Rena Shinomiya
 **************************************************************/

#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

// Standard library
#include <stdbool.h>    // bool

// This project
#include "image.h"  // IMAGE
#include "scene.h"  // SCENE
#include "color.h"  // COLOR
#include "shape.h"  // COLLISION

//*************************************************************
/// Distance of viewing plane from eye.
#define VIEW_DISTANCE 1.0

/// Considered completely dark if under this value because 0.003 < (1 / 255).
#define SHADOW_THRESHOLD 0.003

/// Only consider collisions greater than this distance for shadow casting.
#define COLLISION_THRESHOLD 0.001

/// The index of refraction of the material containing the eye.
#define INITIAL_REFRACTION 1.0

/// The maximum recursion depth for recursive ray tracing.
#define RECURSION_DEPTH 5

/**********************************************************//**
 * @brief Render the SCENE using a raytracing algorithm
 * @param image: The image to write to
 * @param scene: The scene to render
 * @return Whether the image was rendered successfully.
 **************************************************************/
extern bool raytrace_Render(IMAGE *image, const SCENE *scene);

/*============================================================*/
#endif // _RAYTRACE_H_
