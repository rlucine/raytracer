/**********************************************************//**
 * @file scene.h
 * @brief Header file for raytracer scenes
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _SCENE_H_
#define _SCENE_H_

// This project
#include "rgb.h"
#include "vector.h"
#include "shape.h"

/*============================================================*
 * Constants
 *============================================================*/

/// @def SUCCESS
/// @brief Returned by PPM functions if the operation succeeded
#define SUCCESS 0

/// @def FAILURE
/// @brief Returned by PPM functions if the operation failed
#define FAILURE -1

/**********************************************************//**
 * @typedef SCENE
 * @brief Struct for storing all scene data
 **************************************************************/
typedef struct {
    POINT eye;      ///< Position of the eye in the scene
    VECTOR view;    ///< Direction the eye is pointing
    VECTOR up;      ///< Rotation of the camera
    double fov;     ///< Field of view in degrees
    int width;      ///< Width of the image plane
    int height;     ///< Height of the image plane
    RGB background; ///< The default background color
    int nshapes;    ///< How many shapes are in the scene
    SHAPE *shapes;  ///< Array of all shapes in the scene
} SCENE;

/**********************************************************//**
 * @brief Load a scene from a file
 * @param scene: An uninitialized SCENE to read data into
 * @param filename: The file to read
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int scene_Decode(SCENE *scene, const char *filename);

/**********************************************************//**
 * @brief Destroy the SCENE structure
 * @param scene: The sturcture to destroy
 **************************************************************/
extern void scene_Destroy(SCENE *scene);

/*============================================================*/
#endif // _SCENE_H_
