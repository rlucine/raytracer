/**********************************************************//**
 * @file raytrace.c
 * @brief Implementation of raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <string.h>     // memcpy
#include <math.h>       // tan, atan ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert

// This project
#include "rgb.h"
#include "vector.h"
#include "ppm.h"
#include "shape.h"
#include "scene.h"
#include "raytrace.h"

/*============================================================*
 * Constants
 *============================================================*/
#define VIEW_DISTANCE 1.0

/*============================================================*
 * Get the viewing plane
 *============================================================*/
static int raytrace_GetView(PLANE *view, const SCENE *scene) {
    
    // Get the aspect ratio
    double aspect = (double)scene_GetWidth(scene) / (double)scene_GetHeight(scene);
    
    // Get the fields of view and plane dimensions
    double fov_vertical = scene_GetFieldOfView(scene);
    double height = 2.0*VIEW_DISTANCE*tan(fov_vertical / 2.0);
    double width = height * aspect;
    double fov_horizontal = 2.0 * atan(width / (2.0 * VIEW_DISTANCE));
    (void)fov_horizontal;
    
    // Get the u basis vector
    vector_Cross(&view->u, scene_GetUpDirection(scene), scene_GetViewDirection(scene));
    vector_Unit(&view->u, &view->u);
    if (vector_IsZero(&view->u)) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_GetView failed: Null u vector\n");
#endif
        return FAILURE;
    }
    
    // Get the v basis vector
    vector_Cross(&view->v, scene_GetViewDirection(scene), &view->u);
    vector_Unit(&view->v, &view->v);
    if (vector_IsZero(&view->v)) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_GetView failed: Null v vector\n");
#endif
        return FAILURE;
    }
    
    // Get the upper left corner of the plane
    VECTOR du, dv, distance;
    vector_Multiply(&du, &view->u, width / -2.0);
    vector_Multiply(&dv, &view->v, height / 2.0);
    vector_Copy(&distance, scene_GetViewDirection(scene));
    vector_Multiply(&distance, &distance, 1.0 / vector_Magnitude(&distance));
    vector_Copy(&view->origin, scene_GetEyePosition(scene));
    vector_Add(&view->origin, &view->origin, &distance);
    vector_Add(&view->origin, &view->origin, &du);
    vector_Add(&view->origin, &view->origin, &dv);
    return SUCCESS;
}

/*============================================================*
 * Cast one ray
 *============================================================*/
static int raytrace_Cast(RGB *color, const LINE *ray, const SCENE *scene) {
    
    // Collision detectors
    COLLISION closest, current;
    closest.how = COLLISION_NONE;
    
    // Check every shape
    const SHAPE *who = NULL;
    const SHAPE *shape;
    int n = 0;
    int nshapes = scene_GetNumberOfShapes(scene);
    while (n < nshapes) {
        // Read the shape data
        shape = scene_GetShape(scene, n);
        if (!shape) {
#ifdef DEBUG
            fprintf(stderr, "raytrace_Cast failed: No shape with identifier %d\n", n);
#endif
            return FAILURE;
        }
        
        // Collide with this shape
        if (shape_Collide(shape, ray, &current) != SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "raytrace_Cast failed: Collision with shape %d failed\n", n);
#endif
            return FAILURE;
        }
        
        // Check distance - closest is either first shape or the current shape
        // if the current shape is closer (but not behind) us.
        if (n == 0 || (current.distance >= 0.0 && current.distance < closest.distance)) {
            memcpy(&closest, &current, sizeof(COLLISION));
            who = shape;
        }
        n++;
    }
    
    // Determine color
    if (who && closest.how == COLLISION_SURFACE) {
        // Collided with the surface of the shape
        const MATERIAL *material = shape_GetMaterial(who);
        memcpy(color, &material->color, sizeof(RGB));
        
    } else {
        // No shapes, no collision, or inside a shape
        memcpy(color, scene_GetBackgroundColor(scene), sizeof(RGB));
    }
    
    // Checked all shapes - valid!
    return SUCCESS;
}

/*============================================================*
 * Generate an image
 *============================================================*/
int raytrace_Render(PPM *ppm, const SCENE *scene) {
    
    // Get the scene view
    PLANE view;
    if (raytrace_GetView(&view, scene) != SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_Render failed: Failed to generate viewing plane\n");
#endif
        return FAILURE;
    }
    
    // Get the PPM output
    if (ppm_Create(ppm, scene_GetWidth(scene), scene_GetHeight(scene)) != SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_Render failed: Failed to create output image\n");
#endif
        return FAILURE;
    }
    
    // Image pixel size
    int width = ppm_GetWidth(ppm);
    int height = ppm_GetHeight(ppm);
    
    // Establish the ray origin and direction
    LINE ray;
    vector_Copy(&ray.origin, scene_GetEyePosition(scene));
    
    // What is the ray target
    VECTOR target;
    vector_Copy(&target, &view.origin);
    
    // Establish the step
    VECTOR dx, dy;
    vector_Multiply(&dx, &view.u, 1.0 / (double)(width - 1));
    vector_Multiply(&dy, &view.v, 1.0 / (double)(height - 1));
    
    // We buffer the step in Y separately because of floating point
    // error if we mathematically go backwards
    VECTOR ystep;
    ystep.x = ystep.y = ystep.z = 0.0;
    
    // Send rays
    RGB color;
    int y = 0;
    int x = 0;
    while (y < height) {
        x = 0;
        while (x < width) {
            // Get the direction from the eye to the target
            vector_Subtract(&ray.direction, &target, scene_GetEyePosition(scene));
            
            // Cast this ray
            if (raytrace_Cast(&color, &ray, scene) != SUCCESS) {
#ifdef DEBUG
                fprintf(stderr, "raytrace_Render failed: Failed to cast ray (%d, %d)\n", x, y);
#endif
                return FAILURE;
            }
            
            // Put the color
            if (ppm_SetPixel(ppm, x, y, &color) != SUCCESS) {
#ifdef DEBUG
                fprintf(stderr, "raytrace_Render failed: Failed to set color at (%d, %d)\n", x, y);
#endif
                return FAILURE;
            }
#ifdef DEBUG
            const RGB *what = ppm_GetPixel(ppm, x, y);
            assert(what->r == color.r);
            assert(what->g == color.g);
            assert(what->b == color.b);
#endif
            
            // Step forward in x
            vector_Add(&target, &target, &dx);
            x++;
        }
        
        // Step forward in y
        vector_Add(&ystep, &ystep, &dy);
        vector_Add(&target, &view.origin, &ystep);
        y++;
    }
    return SUCCESS;
}

/*============================================================*/