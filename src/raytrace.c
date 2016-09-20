/**********************************************************//**
 * @file raytrace.c
 * @brief Implementation of raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <string.h>     // memcpy
#include <math.h>       // tan, atan, INFINITY ...
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

typedef struct {
    POINT origin;
    VECTOR u;
    VECTOR v;
    double width;
    double height;
} VIEWPLANE;

/*============================================================*
 * Get the viewing plane
 *============================================================*/
static int raytrace_GetView(VIEWPLANE *view, const SCENE *scene) {
    
    // Get the aspect ratio
    double aspect = (double)scene_GetWidth(scene) / (double)scene_GetHeight(scene);
    
    // Get the fields of view and plane dimensions
    double fov_vertical = M_PI * scene_GetFieldOfView(scene) / 180.0;
    double height = 2.0*VIEW_DISTANCE*tan(fov_vertical / 2.0);
    double width = height * aspect;
    double fov_horizontal = 2.0 * atan(width / (2.0 * VIEW_DISTANCE));
    (void)fov_horizontal;
    
    // Get the u basis vector
    vector_Cross(&view->u, scene_GetViewDirection(scene), scene_GetUpDirection(scene));
    vector_Unit(&view->u, &view->u);
    if (vector_IsZero(&view->u)) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_GetView failed: Null u vector (%lf, %lf, %lf)\n", view->u.x, view->u.y, view->u.z);
#endif
        return FAILURE;
    }
    
    // Get the v basis vector
    vector_Cross(&view->v, &view->u, scene_GetViewDirection(scene));
    vector_Unit(&view->v, &view->v);
    if (vector_IsZero(&view->v)) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_GetView failed: Null v vector (%lf, %lf, %lf)\n", view->v.x, view->v.y, view->v.z);
        fprintf(stderr, "raytrace_GetView failed: U is (%lf, %lf, %lf)\n", view->u.x, view->u.y, view->u.z);
#endif
        return FAILURE;
    }
    
    // Get the upper left corner of the plane
    VECTOR du, dv, distance;
    vector_Multiply(&du, &view->u, width / -2.0);
    vector_Multiply(&dv, &view->v, height / 2.0);
    vector_Copy(&distance, scene_GetViewDirection(scene));
    vector_Unit(&distance, scene_GetViewDirection(scene));
    vector_Multiply(&distance, &distance, VIEW_DISTANCE / vector_Magnitude(&distance));
    vector_Copy(&view->origin, scene_GetEyePosition(scene));
    vector_Add(&view->origin, &view->origin, &distance);
    vector_Add(&view->origin, &view->origin, &du);
    vector_Add(&view->origin, &view->origin, &dv);
    
    // Scale basis vectors
    view->width = width;
    view->height = height;
    return SUCCESS;
}

/*============================================================*
 * Shader
 *============================================================*/
static int raytrace_Shade(RGB *color, const MATERIAL *material) {
    memcpy(color, &material->color, sizeof(RGB));
    return SUCCESS;
}

/*============================================================*
 * Cast one ray
 *============================================================*/
static int raytrace_Cast(RGB *color, const LINE *ray, const SCENE *scene) {
    
    // Collision detectors
    COLLISION closest, current;
    closest.how = COLLISION_NONE;
    closest.distance = INFINITY;
    
    // Check every shape
    int who = -1;
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
        switch(current.how) {
        case COLLISION_SURFACE:
        case COLLISION_INSIDE:
            if (n == 0 || (current.distance >= 0.0 && current.distance < closest.distance)) {
                memcpy(&closest, &current, sizeof(COLLISION));
                who = n;
            }
            break;
        
        case COLLISION_NONE:
        default:
            break;
        }
        n++;
    }
    
#ifdef DEBUG
    switch (closest.how) {
    case COLLISION_SURFACE:
        fprintf(stderr, "raytrace_Cast: Collided with %d at distance %lf\n", who, closest.distance);
        break;
    
    case COLLISION_INSIDE:
        fprintf(stderr, "raytrace_Cast: Point is inside object %d\n", who);
        break;
    
    case COLLISION_NONE:
    default:
        fprintf(stderr, "raytrace_Cast: No collision\n");
        break;
    }
#endif
    
    // Determine color
    if (who >= 0 && closest.how == COLLISION_SURFACE) {
        // Collided with the surface of the shape
        const SHAPE *target = scene_GetShape(scene, who);
        const MATERIAL *material = shape_GetMaterial(target);
        if (raytrace_Shade(color, material) != SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "raytrace_Cast: Shader failed\n");
#endif
            return FAILURE;
        }
        
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
    VIEWPLANE view;
    if (raytrace_GetView(&view, scene) != SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "raytrace_Render failed: Failed to generate viewing plane\n");
#endif
        return FAILURE;
    }
    
#ifdef DEBUG
    fprintf(stderr, "raytrace_Render: Viewing plane origin is (%lf, %lf, %lf)\n", view.origin.x, view.origin.y, view.origin.z);
    fprintf(stderr, "raytrace_Render: Viewing plane u is (%lf, %lf, %lf)\n", view.u.x, view.u.y, view.u.z);
    fprintf(stderr, "raytrace_Render: Viewing plane v is (%lf, %lf, %lf)\n", view.v.x, view.v.y, view.v.z);
#endif
    
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
    vector_Multiply(&dx, &view.u, view.width / (scene_GetWidth(scene) - 1));
    
    // Negate v as we are moving from upper left to lower right
    vector_Multiply(&dy, &view.v, -view.height / (scene_GetHeight(scene) - 1));
    
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
#ifdef DEBUG
            fprintf(stderr, "raytrace_Render: At pixel (%d, %d)\n", x, y);
            fprintf(stderr, "raytrace_Render: At point (%lf, %lf, %lf)\n", target.x, target.y, target.z);
#endif

            // Get the direction from the eye to the target
            vector_Subtract(&ray.direction, &target, scene_GetEyePosition(scene));
            vector_Unit(&ray.direction, &ray.direction);
            
#ifdef DEBUG
            fprintf(stderr, "raytrace_Render: Casting in direction (%lf, %lf, %lf)\n", ray.direction.x, ray.direction.y, ray.direction.z);
#endif
            
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
