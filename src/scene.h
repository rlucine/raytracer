/**********************************************************//**
 * @file scene.h
 * @brief Header file for raytracer scenes
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _SCENE_H_
#define _SCENE_H_

// This project
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "light.h"      // LIGHT

/**********************************************************//**
 * @def MIN_FOV
 * @brief The minimum field of view
 **************************************************************/
#define MIN_FOV 0.0

/**********************************************************//**
 * @def MAX_FOV
 * @brief The maximum field of view
 **************************************************************/
#define MAX_FOV 180.0

/**********************************************************//**
 * @def PROJECT_PARALLEL
 * @brief Render the scene with paralllel projection
 **************************************************************/
#define PROJECT_PARALLEL 1

/**********************************************************//**
 * @def PROJECT_PERSPECTIVE
 * @brief Render the scene with perspective projection
 **************************************************************/
#define PROJECT_PERSPECTIVE 0

/**********************************************************//**
 * @struct SCENE
 * @brief Struct for storing all scene data
 **************************************************************/
typedef struct {
    // Bit flags
    int flags;      ///< Rendering flags
    
    // Required components
    POINT eye;          ///< Position of the eye in the scene
    VECTOR view;        ///< Direction the eye is pointing
    VECTOR up;          ///< Rotation of the camera
    double fov;         ///< Field of view in degrees
    int width;          ///< Width of the image plane
    int height;         ///< Height of the image plane
    COLOR background;   ///< The default background color
    
    // Shapes
    int nshapes;    ///< How many shapes are in the scene
    SHAPE *shapes;  ///< Array of all shapes in the scene
    
    // Lights
    int nlights;    ///< How many lights are in the scene
    LIGHT *lights;  ///< Array of all lights in the scene
    
    // Onboard data storage for shapes
    int nmaterials;         ///< How many materials?
    MATERIAL *materials;    ///< Buffer shared SHAPE materials here
    int ntextures;          ///< How many textures?
    TEXTURE *textures;      ///< Buffer shared TEXTURE here
} SCENE;

/**********************************************************//**
 * @brief Get the eye position in the scene
 * @param scene: The scene to read
 * @return Pointer to the eye position
 **************************************************************/
extern const POINT *scene_GetEyePosition(const SCENE *scene);

/**********************************************************//**
 * @brief Get the view direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the view direction. The view direction
 * may not be normalized and may not be orthogonal to the up.
 **************************************************************/
extern const VECTOR *scene_GetViewDirection(const SCENE *scene);

/**********************************************************//**
 * @brief Get the up direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the up direction. The up direction may
 * not be normalized and may not be orthogonal to the view.
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
 * @return The width of the scene in pixels
 **************************************************************/
extern int scene_GetWidth(const SCENE *scene);

/**********************************************************//**
 * @brief Get the scene height in pixels
 * @param scene: The scene to read
 * @return The height of the scene in pixels
 **************************************************************/
extern int scene_GetHeight(const SCENE *scene);

/**********************************************************//**
 * @brief Get the background color of the scene
 * @param scene: The scene to read
 * @return Pointer to the background color
 **************************************************************/
extern const COLOR *scene_GetBackgroundColor(const SCENE *scene);

/**********************************************************//**
 * @brief Get how many shapes are in the scene
 * @param scene: The scene to read
 * @return The number of shapes in the scene. scene_GetShape
 * is guaranteed not to fail for non-negative int less than
 * this returned value.
 **************************************************************/
extern int scene_GetNumberOfShapes(const SCENE *scene);

/**********************************************************//**
 * @brief Get the shape from the scene
 * @param scene: The scene to read
 * @param index: Shape index from 0 to the number of shapes
 * minus one.
 * @return Pointer to the shape
 **************************************************************/
extern const SHAPE *scene_GetShape(const SCENE *scene, int index);

/**********************************************************//**
 * @brief Get how many lights are in the scene
 * @param scene: The scene to read
 * @return The number of lights in the scene. scene_GetLight
 * is guaranteed not to fail for non-negative int less than
 * this returned value.
 **************************************************************/
extern int scene_GetNumberOfLights(const SCENE *scene);

/**********************************************************//**
 * @brief Get the light from the scene
 * @param scene: The scene to read
 * @param index: Light index from 0 to the number of lights
 * minus one.
 * @return Pointer to the light
 **************************************************************/
extern const LIGHT *scene_GetLight(const SCENE *scene, int index);

/**********************************************************//**
 * @brief Destroy the SCENE structure. The structure cannot
 * have been destroyed already, and must have been initialized.
 * @param scene: The sturcture to destroy. The structure may
 * not be used after calling this function.
 **************************************************************/
extern void scene_Destroy(SCENE *scene);

/*============================================================*/
#endif // _SCENE_H_
