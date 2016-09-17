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
/// @brief Returned by decoder functions if the operation succeeded
#define SUCCESS 0

/// @def FAILURE
/// @brief Returned by decoder functions if the operation failed
#define FAILURE -1

/// @def NO_MATCH
/// @brief Returned by decoder functions if no patterns matched
#define NO_MATCH -2

/// @def MIN_FOV
/// @brief The minimum field of view
#define MIN_FOV 0.0

/// @def MAX_FOV
/// @brief The maximum field of view
#define MAX_FOV 180.0

/**********************************************************//**
 * @typedef SCENE_FLAG
 * @brief Bitset flags for which scene fields have been found
 **************************************************************/
typedef enum {
    FLAG_EYE = 1,
    FLAG_VIEW = 2,
    FLAG_UP = 4,
    FLAG_FOV = 8,
    FLAG_SIZE = 16,
    FLAG_BACKGROUND = 32,
    FLAG_MATERIAL = 64,
} SCENE_FLAG;

/**********************************************************//**
 * @typedef SCENE
 * @brief Struct for storing all scene data
 **************************************************************/
typedef struct {
    SCENE_FLAG flags;   ///< Bitset of which members are initialized
    POINT eye;          ///< Position of the eye in the scene
    VECTOR view;        ///< Direction the eye is pointing
    VECTOR up;          ///< Rotation of the camera
    double fov;         ///< Field of view in degrees
    int width;          ///< Width of the image plane
    int height;         ///< Height of the image plane
    RGB background;     ///< The default background color
    int nshapes;        ///< How many shapes are in the scene
    SHAPE *shapes;      ///< Array of all shapes in the scene
} SCENE;

/**********************************************************//**
 * @brief Load a scene from a file
 * @param scene: An uninitialized SCENE to read data into
 * @param filename: The file to read
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int scene_Decode(SCENE *scene, const char *filename);

/**********************************************************//**
 * @brief Get the eye position in the scene
 * @param scene: The scene to read
 * @return Pointer to the eye position
 **************************************************************/
extern const POINT *scene_GetEyePosition(const SCENE *scene);

/**********************************************************//**
 * @brief Get the view direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the view direction
 **************************************************************/
extern const VECTOR *scene_GetViewDirection(const SCENE *scene);

/**********************************************************//**
 * @brief Get the up direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the up direction
 **************************************************************/
extern const VECTOR *scene_GetUpDirection(const SCENE *scene);

/**********************************************************//**
 * @brief Get the FOV in degrees
 * @param scene: The scene to read
 * @return The field of view in degrees
 **************************************************************/
extern double scene_GetFieldOfView(const SCENE *scene);

/**********************************************************//**
 * @brief Get the scene width in pixels
 * @param scene: The scene to read
 * @return The width of the scene
 **************************************************************/
extern int scene_GetWidth(const SCENE *scene);

/**********************************************************//**
 * @brief Get the scene height in pixels
 * @param scene: The scene to read
 * @return The height of the scene
 **************************************************************/
extern int scene_GetHeight(const SCENE *scene);

/**********************************************************//**
 * @brief Get the background RGB color of the scene
 * @param scene: The scene to read
 * @return Pointer to the background color
 **************************************************************/
extern const RGB *scene_GetBackgroundColor(const SCENE *scene);

/**********************************************************//**
 * @brief Get how many shapes are in the scene
 * @param scene: The scene to read
 * @return The number of shapes in the scene
 **************************************************************/
extern int scene_GetNumberOfShapes(const SCENE *scene);

/**********************************************************//**
 * @brief Get the shape from the scene
 * @param scene: The scene to read
 * @param index: Shape index from 0 to the number of shapes - 1
 * @return Pointer to the shape
 **************************************************************/
extern const SHAPE *scene_GetShape(const SCENE *scene, int index);

/**********************************************************//**
 * @brief Destroy the SCENE structure
 * @param scene: The sturcture to destroy
 **************************************************************/
extern void scene_Destroy(SCENE *scene);

/*============================================================*/
#endif // _SCENE_H_
