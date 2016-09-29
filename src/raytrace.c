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
#include <float.h>      // DBL_EPSILON

// This project
#include "image.h"
#include "vector.h"
#include "shape.h"
#include "scene.h"
#include "raytrace.h"

/*============================================================*
 * Constants
 *============================================================*/
#define VIEW_DISTANCE 1.0

typedef struct {
    POINT origin;   // Upper left corner
    VECTOR u;       // Unit vector u
    VECTOR v;       // Unit vector v
    double width;   // Real width of the viewing plane
    double height;  // Real height of the viewing plane
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
    vector_Normalize(&view->u, &view->u);
    if (vector_IsZero(&view->u)) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_GetView failed: Null u vector (%lf, %lf, %lf)\n", view->u.x, view->u.y, view->u.z);
#endif
        return FAILURE;
    }
    
    // Get the v basis vector
    vector_Cross(&view->v, &view->u, scene_GetViewDirection(scene));
    vector_Normalize(&view->v, &view->v);
    if (vector_IsZero(&view->v)) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_GetView failed: Null v vector (%lf, %lf, %lf)\n", view->v.x, view->v.y, view->v.z);
#ifdef DEBUG
        fprintf(stderr, "raytrace_GetView failed: U is (%lf, %lf, %lf)\n", view->u.x, view->u.y, view->u.z);
#endif
#endif
        return FAILURE;
    }
    
    // Get the upper left corner of the plane
    VECTOR du, dv, distance;
    vector_Multiply(&du, &view->u, width / -2.0);
    vector_Multiply(&dv, &view->v, height / 2.0);
    vector_Copy(&distance, scene_GetViewDirection(scene));
    vector_Normalize(&distance, scene_GetViewDirection(scene));
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
 * Cast one ray
 *============================================================*/
static int raytrace_Cast(COLLISION *closest, const LINE *ray, const SCENE *scene) {
    
    // Collision detectors
    COLLISION current;
    closest->how = COLLISION_NONE;
    closest->distance = INFINITY;
    closest->material = NULL;
    
    // Check every shape
    int who = -1;
    const SHAPE *shape;
    int n = 0;
    int nshapes = scene_GetNumberOfShapes(scene);
    while (n < nshapes) {
        // Read the shape data
        shape = scene_GetShape(scene, n);
        if (!shape) {
#ifdef VERBOSE
            fprintf(stderr, "raytrace_Cast failed: No shape with identifier %d\n", n);
#endif
            return FAILURE;
        }
        
        // Collide with this shape
        if (shape_Collide(shape, ray, &current) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "raytrace_Cast failed: Collision with shape %d failed\n", n);
#endif
            return FAILURE;
        }
        
        // Check distance - closest is either first shape or the current shape
        // if the current shape is closer (but not behind) us.
        switch(current.how) {
        case COLLISION_SURFACE:
        case COLLISION_INSIDE:
            if (n == 0 || (current.distance >= 0.0 && current.distance < closest->distance)) {
                memcpy(closest, &current, sizeof(COLLISION));
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
    switch (closest->how) {
    case COLLISION_SURFACE:
        fprintf(stderr, "raytrace_Cast: Collided with %d at (%lf, %lf, %lf)\n", who, closest->where.x, closest->where.y, closest->where.z);
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

    // Get the material properties
    if (who >= 0 && closest->how != COLLISION_NONE) {
        // Collided with the surface of the shape
        const SHAPE *target = scene_GetShape(scene, who);
        closest->material = shape_GetMaterial(target);
    }

    // Checked all shapes - valid!
    return SUCCESS;
}

/*============================================================*
 * Shadowing
 *============================================================*/
#ifdef SHADOWS
static int raytrace_Shadow(double *shadows, const POINT *where, const LIGHT *light, const SCENE *scene) {
    
    // Set up ray pointing to light
    LINE ray;
    double distance;
    memcpy(&ray.origin, where, sizeof(POINT));
    if (light_GetDirection(light, where, &ray.direction, &distance) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_Shadow failed: Invalid light\n");
#endif
        return FAILURE;
    }
    
    // Fire the ray
    COLLISION collision;
    if (raytrace_Cast(&collision, &ray, scene) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_Shadow failed: Failed to shoot shadow ray\n");
#endif
        return FAILURE;
    }
    
    // Check collisions
    if (collision.how != COLLISION_NONE && collision.distance < distance && collision.distance > DBL_EPSILON) {
        // Something in between the light and us, and it isn't ourself!
        *shadows = 0.0;
        return SUCCESS;
    }
    *shadows = 1.0;
    return SUCCESS;
}
#endif

/*============================================================*
 * Shader
 *============================================================*/
static int raytrace_Shade(COLOR *color, const COLLISION *collision, const SCENE *scene) {
    
    // Set the ambient color of the object
    const MATERIAL *material = collision->material;
    vector_Multiply(color, &material->color, material->ambient);
    
    // Add up component for each light
    COLOR temp;
#ifdef SHADOWS
    double shadows;
#endif
    int i;
    int max = scene_GetNumberOfLights(scene);
    const LIGHT *light;
    for (i = 0; i < max; i++) {
        // Check for shadows
        light = scene_GetLight(scene, i);
        
#ifdef SHADOWS
        if (raytrace_Shadow(&shadows, &collision->where, light, scene) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "raytrace_Shade failed: Failed to check shadows\n");
#endif
            return FAILURE;
        }
        
        // No contribution from this light
        // TODO check float error
        if (shadows == 0.0) {
#ifdef DEBUG
            fprintf(stderr, "raytrace_Shade: Light obstructed\n");
#endif
            continue;
        }
#endif
        
        // Get shading for this light
        if (light_BlinnPhongShade(light, collision, &scene->eye, &temp) != SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "raytrace_Shade: Shape outside of light area\n");
#endif
            continue;
        }
        
        // Scale light by shadows
#ifdef SHADOWS
        vector_Multiply(&temp, &temp, shadows);
#endif
        
        // Add light contributions
        vector_Add(color, color, &temp);
    }
    
    // Added contribution for every light
    color_Clamp(color);
    return SUCCESS;
}

/*============================================================*
 * Generate an image
 *============================================================*/
int raytrace_Render(IMAGE *image, const SCENE *scene) {
    
    // Get the scene view
    VIEWPLANE view;
    if (raytrace_GetView(&view, scene) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_Render failed: Failed to generate viewing plane\n");
#endif
        return FAILURE;
    }
    
#ifdef DEBUG
    fprintf(stderr, "raytrace_Render: Viewing plane origin is (%lf, %lf, %lf)\n", view.origin.x, view.origin.y, view.origin.z);
    fprintf(stderr, "raytrace_Render: Viewing plane u is (%lf, %lf, %lf)\n", view.u.x, view.u.y, view.u.z);
    fprintf(stderr, "raytrace_Render: Viewing plane v is (%lf, %lf, %lf)\n", view.v.x, view.v.y, view.v.z);
#endif
    
    // Get the image output
    if (image_Create(image, scene_GetWidth(scene), scene_GetHeight(scene)) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "raytrace_Render failed: Failed to create output image\n");
#endif
        return FAILURE;
    }
    
    // Image pixel size
    int width = image_GetWidth(image);
    int height = image_GetHeight(image);
    
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
    COLLISION collision;
    COLOR color;
    RGB rgb;
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
            vector_Normalize(&ray.direction, &ray.direction);
            
#ifdef DEBUG
            fprintf(stderr, "raytrace_Render: Casting in direction (%lf, %lf, %lf)\n", ray.direction.x, ray.direction.y, ray.direction.z);
#endif
            
            // Cast this ray
            if (raytrace_Cast(&collision, &ray, scene) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "raytrace_Render failed: Failed to cast ray (%d, %d)\n", x, y);
#endif
                return FAILURE;
            }
            
            // Determine color
            if (collision.how != COLLISION_NONE) {
                // Collided with the surface of the shape
                if (raytrace_Shade(&color, &collision, scene) != SUCCESS) {
        #ifdef VERBOSE
                    fprintf(stderr, "raytrace_Cast: Shader failed\n");
        #endif
                    return FAILURE;
                }
                
                // Get the RGB color
                color_ToRgb(&rgb, &color);
                
            } else {
                // No shapes, no collision, or inside a shape
                color_ToRgb(&rgb, scene_GetBackgroundColor(scene));
            }
            
            // Put the color
            if (image_SetPixel(image, x, y, &rgb) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "raytrace_Render failed: Failed to set color at (%d, %d)\n", x, y);
#endif
                return FAILURE;
            }
#ifdef DEBUG
            const RGB *what = image_GetPixel(image, x, y);
            assert(what->r == rgb.r);
            assert(what->g == rgb.g);
            assert(what->b == rgb.b);
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
