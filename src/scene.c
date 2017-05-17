/**********************************************************//**
 * @file scene.c
 * @brief Implementation file for raytracer scenes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // free

// This project
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "light.h"      // LIGHT
#include "mesh.h"       // MESH

// Debugging modules
#include "debug.h"      // eprintf, assert

// Included files
#include "parser.inc"

/*============================================================*
 * Scene destructor
 *============================================================*/
void scene_Destroy(SCENE *scene) {
    // Free all the allocated shape data
    int i;
    
    // Free allocated array
    if (scene->nshapes > 0 && scene->shapes) {
        for (i = 0; i < scene->nshapes; i++) {
            shape_Destroy(scene->shapes[i]);
            free(scene->shapes[i]);
        }
        free(scene->shapes);
        scene->nshapes = 0;
        scene->shapes = NULL;
    }
    
    // Free all the lights
    if (scene->nlights > 0 && scene->lights) {
        for (i = 0; i < scene->nlights; i++) {
            free(scene->lights[i]);
        }
        free(scene->lights);
        scene->nlights = 0;
        scene->lights = NULL;
    }
    
    // Free all the materials
    if (scene->nmaterials > 0 && scene->materials) {
        for (i = 0; i < scene->nmaterials; i++) {
            free(scene->materials[i]);
        }
        free(scene->materials);
        scene->nmaterials = 0;
        scene->materials = NULL;
    }
    
    // Free all the textures
    if (scene->ntextures > 0 && scene->textures) {
        for (i = 0; i < scene->ntextures; i++) {
            image_Destroy(scene->textures[i]);
            free(scene->textures[i]);
        }
        free(scene->textures);
        scene->ntextures = 0;
        scene->textures = NULL;
    }
    
    // Free all the mesh data
    mesh_Destroy(&scene->mesh);
}

/*============================================================*/
