/**********************************************************//**
 * @file raytrace.h
 * @brief Header file for raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _RAYTRACE_H_
#define _RAYTRACE_H_

// This project
#include "macro.h"
#include "image.h"
#include "scene.h"

/**********************************************************//**
 * @brief Render the SCENE using a raytracing algorithm
 * @param image: The image to write to
 * @param scene: The scene to render
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int raytrace_Render(IMAGE *image, const SCENE *scene);

/*============================================================*/
#endif // _RAYTRACE_H_