/**********************************************************//**
 * @file scene.c
 * @brief Implementation file for raytracer scenes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, realloc, size_t ...
#include <stdio.h>      // fprintf, stderr, fopen, fclose ...
#include <string.h>     // strcmp
#include <ctype.h>      // isspace

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "scene.h"      // SCENE
#include "light.h"      // LIGHT

// Debugging modules
#ifdef DEBUG
#include "tracemalloc.h"
#endif

// Included files
#include "parser.inc"

/*============================================================*
 * Scene getters
 *============================================================*/
const POINT *scene_GetEyePosition(const SCENE *scene) {
    return &scene->eye;
}

const VECTOR *scene_GetViewDirection(const SCENE *scene) {
    return &scene->view;
}

const VECTOR *scene_GetUpDirection(const SCENE *scene) {
    return &scene->up;
}

double scene_GetFieldOfView(const SCENE *scene) {
    return scene->fov;
}

int scene_GetWidth(const SCENE *scene) {
    return scene->width;
}

int scene_GetHeight(const SCENE *scene) {
    return scene->height;
}

const COLOR *scene_GetBackgroundColor(const SCENE *scene) {
    return &scene->background;
}

int scene_GetNumberOfShapes(const SCENE *scene) {
    return scene->nshapes;
}

const SHAPE *scene_GetShape(const SCENE *scene, int index) {
    return &scene->shapes[index];
}

int scene_GetNumberOfLights(const SCENE *scene) {
    return scene->nlights;
}

const LIGHT *scene_GetLight(const SCENE *scene, int index) {
    return &scene->lights[index];
}

/*============================================================*
 * Scene destructor
 *============================================================*/
void scene_Destroy(SCENE *scene) {
    
    // Free all the allocated shape data
    int i;
    
    // Free allocated array
    if (scene->nshapes > 0 && scene->shapes) {
        for (i = 0; i < scene->nshapes; i++) {
            shape_Destroy(&scene->shapes[i]);
        }
        free(scene->shapes);
        scene->nshapes = 0;
        scene->shapes = NULL;
    }
    
    // Free all the lights
    if (scene->nlights > 0 && scene->lights) {
        free(scene->lights);
        scene->nlights = 0;
        scene->lights = NULL;
    }
    
    // Free all the materials
    if (scene->nmaterials > 0 && scene->materials) {
        free(scene->materials);
        scene->nmaterials = 0;
        scene->materials = NULL;
    }
    
    // Free all the textures
    if (scene->ntextures > 0 && scene->textures) {
        for (i = 0; i < scene->ntextures; i++) {
            image_Destroy(&scene->textures[i]);
        }
        free(scene->textures);
        scene->ntextures = 0;
        scene->textures = NULL;
    }
}

/*============================================================*/
