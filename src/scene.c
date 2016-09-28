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
#include "image.h"
#include "vector.h"
#include "shape.h"
#include "scene.h"
#include "tracemalloc.h"

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

/*============================================================*
 * Scene destructor
 *============================================================*/
void scene_Destroy(SCENE *scene) {
    int i;
    
    // Free all the allocated shape data
    for (i = 0; i < scene->nshapes; i++) {
       shape_Destroy(&scene->shapes[i]);
    }
    
    // Free allocated array
    if (scene->shapes) {
       free(scene->shapes);
    }
}

/*============================================================*/
