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

/**********************************************************//**
 * @brief Render the SCENE using a raytracing algorithm
 * @param image: The image to write to
 * @param scene: The scene to render
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int raytrace_Render(IMAGE *image, const SCENE *scene);

/*============================================================*/
#endif // _RAYTRACE_H_