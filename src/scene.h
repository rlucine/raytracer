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
#include "mesh.h"       // MESH

//*************************************************************
#define MIN_FOV 0.0     ///< The minimum field of view.
#define MAX_FOV 180.0   ///< The maximum field of view.

/**********************************************************//**
 * @struct SCENE
 * @brief Struct for storing all scene data
 **************************************************************/
typedef struct {
    // Required components
    VECTOR eye;          ///< Position of the eye in the scene
    VECTOR view;        ///< Direction the eye is pointing
    VECTOR up;          ///< Rotation of the camera
    float fov;         ///< Field of view in degrees
    int width;          ///< Width of the image plane
    int height;         ///< Height of the image plane
    COLOR background;   ///< The default background color
    
    // Shapes
    int nshapes;        ///< How many shapes are in the scene
    SHAPE **shapes;     ///< Array of all shapes in the scene
    MESH mesh;          ///< Container for all FACE data
    
    // Lights
    int nlights;        ///< How many lights are in the scene
    LIGHT **lights;     ///< Array of all lights in the scene
    
    // Onboard data storage for shapes
    int nmaterials;         ///< How many materials?
    MATERIAL **materials;   ///< Buffer shared SHAPE materials here
    int ntextures;          ///< How many textures?
    TEXTURE **textures;     ///< Buffer shared TEXTURE here
} SCENE;

/**********************************************************//**
 * @brief Load a scene from a file. If this function succeeds
 * you must destroy the SCENE with scene_Destroy. If this
 * function fails all memory usage is cleaned up and you will
 * not need to use scene_Destroy.
 * @param scene: An uninitialized SCENE to read data into
 * @param filename: The file to read data from
 * @return Whether the decode worked.
 **************************************************************/
extern bool scene_Decode(SCENE *scene, const char *filename);

/**********************************************************//**
 * @brief Get the eye position in the scene
 * @param scene: The scene to read
 * @return Pointer to the eye position
 **************************************************************/
static inline const VECTOR *scene_GetEyePosition(const SCENE *scene) {
    return &scene->eye;
}

/**********************************************************//**
 * @brief Get the view direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the view direction. The view direction
 * may not be normalized and may not be orthogonal to the up.
 **************************************************************/
static inline const VECTOR *scene_GetViewDirection(const SCENE *scene) {
    return &scene->view;
}

/**********************************************************//**
 * @brief Get the up direction in the scene
 * @param scene: The scene to read
 * @return Pointer to the up direction. The up direction may
 * not be normalized and may not be orthogonal to the view.
 **************************************************************/
static inline const VECTOR *scene_GetUpDirection(const SCENE *scene) {
    return &scene->up;
}

/**********************************************************//**
 * @brief Get the FOV in degrees
 * @param scene: The scene to read
 * @return The field of view in degrees
 **************************************************************/
static inline float scene_GetFieldOfView(const SCENE *scene) {
    return scene->fov;
}

/**********************************************************//**
 * @brief Get the scene width in pixels
 * @param scene: The scene to read
 * @return The width of the scene in pixels
 **************************************************************/
static inline int scene_GetWidth(const SCENE *scene) {
    return scene->width;
}

/**********************************************************//**
 * @brief Get the scene height in pixels
 * @param scene: The scene to read
 * @return The height of the scene in pixels
 **************************************************************/
static inline int scene_GetHeight(const SCENE *scene) {
    return scene->height;
}

/**********************************************************//**
 * @brief Get the background color of the scene
 * @param scene: The scene to read
 * @return Pointer to the background color
 **************************************************************/
static inline const COLOR *scene_GetBackgroundColor(const SCENE *scene) {
    return &scene->background;
}

/**********************************************************//**
 * @brief Get how many shapes are in the scene
 * @param scene: The scene to read
 * @return The number of shapes in the scene. scene_GetShape
 * is guaranteed not to fail for non-negative int less than
 * this returned value.
 **************************************************************/
static inline int scene_GetNumberOfShapes(const SCENE *scene) {
    return scene->nshapes;
}

/**********************************************************//**
 * @brief Get the shape from the scene
 * @param scene: The scene to read
 * @param index: Shape index from 0 to the number of shapes
 * minus one.
 * @return Pointer to the shape
 **************************************************************/
static inline const SHAPE *scene_GetShape(const SCENE *scene, int index) {
    return scene->shapes[index];
}

/**********************************************************//**
 * @brief Get how many lights are in the scene
 * @param scene: The scene to read
 * @return The number of lights in the scene. scene_GetLight
 * is guaranteed not to fail for non-negative int less than
 * this returned value.
 **************************************************************/
static inline int scene_GetNumberOfLights(const SCENE *scene) {
    return scene->nlights;
}

/**********************************************************//**
 * @brief Get the light from the scene
 * @param scene: The scene to read
 * @param index: Light index from 0 to the number of lights
 * minus one.
 * @return Pointer to the light
 **************************************************************/
static inline const LIGHT *scene_GetLight(const SCENE *scene, int index) {
    return scene->lights[index];
}

/**********************************************************//**
 * @brief Destroy the SCENE structure. The structure cannot
 * have been destroyed already, and must have been initialized.
 * @param scene: The sturcture to destroy. The structure may
 * not be used after calling this function.
 **************************************************************/
extern void scene_Destroy(SCENE *scene);

/*============================================================*/
#endif // _SCENE_H_
