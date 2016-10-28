/**********************************************************//**
 * @file raytrace.c
 * @brief Implementation of raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // rand, RAND_MAX
#include <string.h>     // memcpy
#include <math.h>       // pow, tan, atan, INFINITY ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "scene.h"      // SCENE
#include "raytrace.h"   // SHADOW_THRESHOLD ...

// Debugging libraries
#include "debug.h"

/*============================================================*
 * Viewing plane
 *============================================================*/
typedef struct {
    POINT origin;   // Upper left corner
    VECTOR u;       // Unit vector u
    VECTOR v;       // Unit vector v
    double width;   // Real width of the viewing plane
    double height;  // Real height of the viewing plane
    POINT center;   // The center of the viewing plane
} VIEWPLANE;

/*============================================================*
 * Get the viewing plane
 *============================================================*/
static int raytrace_GetView(VIEWPLANE *view, double view_distance, const SCENE *scene) {
    
    // Get the aspect ratio
    double aspect = (double)scene_GetWidth(scene) / (double)scene_GetHeight(scene);
    
    // Get the fields of view and plane dimensions
    double fov_vertical = M_PI * scene_GetFieldOfView(scene) / 180.0;
    double height = 2.0*tan(fov_vertical / 2.0);
    double width = height * aspect;
    
    // Get the u basis vector
    vector_Cross(&view->u, scene_GetViewDirection(scene), scene_GetUpDirection(scene));
    vector_Normalize(&view->u, &view->u);
    if (vector_IsZero(&view->u)) {
        errmsg("Null u vector (%lf, %lf, %lf)\n", view->u.x, view->u.y, view->u.z);
        return FAILURE;
    }
    
    // Get the v basis vector
    vector_Cross(&view->v, &view->u, scene_GetViewDirection(scene));
    vector_Normalize(&view->v, &view->v);
    if (vector_IsZero(&view->v)) {
        errmsg("Null v vector (%lf, %lf, %lf)\n", view->v.x, view->v.y, view->v.z);
        return FAILURE;
    }
    
    // Get the offset to the upper left
    VECTOR du, dv, distance;
    vector_Multiply(&du, &view->u, width / -2.0);
    vector_Multiply(&dv, &view->v, height / 2.0);
    
    // Get the distance to the viewing plane
    vector_Normalize(&distance, scene_GetViewDirection(scene));
    vector_Multiply(&distance, &distance, view_distance);
    
    // Get the center point of the viewing plane
    vector_Add(&view->center, scene_GetEyePosition(scene), &distance);
    
    // Get the upper left corner
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
            errmsg("No shape with identifier %d\n", n);
            return FAILURE;
        }
        
        // Collide with this shape
        if (shape_Collide(shape, ray, &current) != SUCCESS) {
            errmsg("Collision with shape %d failed\n", n);
            return FAILURE;
        }
        
        // Check distance - closest is either first shape or the current shape
        // if the current shape is closer (but not behind) us.
        if (current.how != COLLISION_NONE && current.distance >= COLLISION_THRESHOLD && (n == 0 || current.distance < closest->distance)) {
            memcpy(closest, &current, sizeof(COLLISION));
            who = n;
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
 * Recursive ray tracing (shadows)
 *============================================================*/
static int raytrace_Shadow(double *shadows, const COLLISION *collision, const LIGHT *light, const SCENE *scene) {
    
    // Set up ray pointing to light
    LINE ray;
    double distance;
    memcpy(&ray.origin, &collision->where, sizeof(POINT));
    if (light_GetDirection(light, &collision->where, &ray.direction, &distance) != SUCCESS) {
        errmsg("Invalid light\n");
        return FAILURE;
    }
    
    // Fire the shadow ray
    COLLISION shadow;
    if (raytrace_Cast(&shadow, &ray, scene) != SUCCESS) {
        errmsg("Failed to shoot shadow ray\n");
        return FAILURE;
    }
    
    // Check collisions
    if ((shadow.how != COLLISION_NONE) && (shadow.distance < distance) && (shadow.distance > COLLISION_THRESHOLD)) {
        // Something in between the light and us, and it isn't ourself!
        *shadows = 0.0;
    } else {
        *shadows = 1.0;
    }
    return SUCCESS;
}

/*============================================================*
 * Recursive ray tracing (reflections)
 *============================================================*/
static int raytrace_Shade(COLOR *, const COLLISION *, const SCENE *, double, int);

static int raytrace_Reflection(COLOR *color, const COLLISION *collision, const SCENE *scene, double irefract, int depth) {
    
    // Stack overflow
    if (depth > RECURSION_DEPTH) {
        vector_Set(color, 0, 0, 0);
        return SUCCESS;
    }
    
    // Get the normal which is in the same direction as the incident
    // vector so we do not have erroneous refraction / reflection
    VECTOR reflection_normal;
    if (vector_Dot(&collision->normal, &collision->incident) < 0) {
        vector_Negate(&reflection_normal, &collision->normal);
    } else {
        vector_Copy(&reflection_normal, &collision->normal);
    }
    
    // Efficient computation of Fresnel reflectance
    const MATERIAL *material = collision->material;
    double cos_theta_i = vector_Dot(&reflection_normal, &collision->incident);
    double pow1 = 1 - cos_theta_i;
    double pow2 = pow1 * pow1;
    double pow5 = pow2 * (pow1 * pow2);
    double fresnel = material->refraction + (1.0 - material->refraction)*pow5;
    
    // Get the reflection ray's direction
    LINE reflection;
    vector_Multiply(&reflection.direction, &reflection_normal, 2.0*vector_Dot(&reflection_normal, &collision->incident));
    vector_Subtract(&reflection.direction, &reflection.direction, &collision->incident);
    
    // The reflection ray's position is the current collision
    vector_Copy(&reflection.origin, &collision->where);

    // Shoot the reflection ray
    COLLISION reflection_collision;
    if (raytrace_Cast(&reflection_collision, &reflection, scene) != SUCCESS) {
        errmsg("Failed to shoot reflection ray\n");
        return FAILURE;
    }

    // Recursively shade the reflection color
    if (reflection_collision.how != COLLISION_NONE) {
        if (raytrace_Shade(color, &reflection_collision, scene, irefract, depth+1) != SUCCESS) {
            errmsg("Failed to shade the reflection ray\n");
            return FAILURE;
        }
        
        // Scale component with reflectivity
        vector_Multiply(color, color, fresnel);
    } else {
        vector_Set(color, 0, 0, 0);
    }
    
    // Early exit if no transparency
    if (fabs(material->opacity - 1.0) < DBL_EPSILON) {
        return SUCCESS;
    }
    
    // Set up the transparency ray direction
    LINE transparency;
    double ratio = irefract / collision->material->refraction;
    vector_Multiply(&transparency.direction, &reflection_normal, -sqrt(1 - ((ratio*ratio)*(1 - cos_theta_i*cos_theta_i))));
    VECTOR temp;
    vector_Multiply(&temp, &reflection_normal, cos_theta_i);
    vector_Subtract(&temp, &temp, &collision->incident);
    vector_Multiply(&temp, &temp, ratio);
    vector_Add(&transparency.direction, &transparency.direction, &temp);
    
    // The transparency ray's location is the current collision
    vector_Copy(&transparency.origin, &collision->where);
    
    // Shoot the transparency ray
    COLLISION transparency_collision;
    if (raytrace_Cast(&transparency_collision, &transparency, scene) != SUCCESS) {
        errmsg("Failed to shoot transparency ray\n");
        return FAILURE;
    }
    
    // Recursively shade transparency color
    if (transparency_collision.how != COLLISION_NONE) {
        COLOR transparency_color;
        if (raytrace_Shade(&transparency_color, &transparency_collision, scene, material->refraction, depth+1) != SUCCESS) {
            errmsg("Failed to shade the transparency ray\n");
            return FAILURE;
        }
        
        // Scale component
        vector_Multiply(&transparency_color, &transparency_color, (1 - fresnel)*(1 - material->opacity));
        vector_Add(color, color, &transparency_color);
    }
    return SUCCESS;
}

/*============================================================*
 * Shader
 *============================================================*/
static int raytrace_Shade(COLOR *color, const COLLISION *collision, const SCENE *scene, double irefract, int depth) {
    
    // Get the diffuse color
    COLOR object_color;
    if (shape_GetColorAt(collision, &object_color) != SUCCESS) {
        errmsg("Failed to get object color\n");
        return FAILURE;
    }
    
    // Set the ambient color of the object
    const MATERIAL *material = collision->material;
    vector_Multiply(color, &object_color, material->ambient);
    
    // Shade color for all lights
    COLOR temp;
    double shadows;
    const LIGHT *light;
    for (int i = 0; i < scene_GetNumberOfLights(scene); i++) {
        // Check for shadows
        light = scene_GetLight(scene, i);
        
        // Get shadows and check float 
        if (raytrace_Shadow(&shadows, collision, light, scene) != SUCCESS) {
            errmsg("Failed to check shadows\n");
            return FAILURE;
        }
        if (shadows < SHADOW_THRESHOLD) {
            continue;
        }
        
        // Get shading for this light
        if (light_BlinnPhongShade(light, collision, &scene->eye, &temp) != SUCCESS) {
            continue;
        }
        vector_Multiply(&temp, &temp, shadows);
        vector_Add(color, color, &temp);
    }
    color_Clamp(color);

    // Recursive ray tracing?
    if (depth < RECURSION_DEPTH) {
        // Determine any reflections
        COLOR reflection_color;
        if (raytrace_Reflection(&reflection_color, collision, scene, irefract, depth) != SUCCESS) {
            errmsg("Failed to get reflection color\n");
            return FAILURE;
        }
        
        // Incorporate the reflected color into the result
        vector_Add(color, color, &reflection_color);
        color_Clamp(color);
    }

    return SUCCESS;
}

/*============================================================*
 * Generate an image
 *============================================================*/
int raytrace_Render(IMAGE *image, const SCENE *scene) {
    
    // Get the scene view
    VIEWPLANE view;
    double distance = VIEW_DISTANCE;
    if (scene->flags & PROJECT_PARALLEL) {
        distance = 0.0;
    }
    if (raytrace_GetView(&view, distance, scene) != SUCCESS) {
        errmsg("Failed to generate viewing plane\n");
        return FAILURE;
    }
    
#ifdef DEBUG
    errmsg("Viewing plane origin is (%lf, %lf, %lf)\n", view.origin.x, view.origin.y, view.origin.z);
    errmsg("Viewing plane u is (%lf, %lf, %lf)\n", view.u.x, view.u.y, view.u.z);
    errmsg("Viewing plane v is (%lf, %lf, %lf)\n", view.v.x, view.v.y, view.v.z);
    errmsg("Viewing plane size is %lf by %lf\n", view.width, view.height);
#endif
    
    // Get the image output
    if (image_Create(image, scene_GetWidth(scene), scene_GetHeight(scene)) != SUCCESS) {
        errmsg("Failed to create output image\n");
        return FAILURE;
    }
    
    // Image pixel size
    int width = image_GetWidth(image);
    int height = image_GetHeight(image);
    
    // This is the ray to shoot
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
            if (scene->flags & PROJECT_PARALLEL) {
                // Parallel direction is always the same
                vector_Copy(&ray.origin, &target);
                vector_Normalize(&ray.direction, scene_GetViewDirection(scene));
            } else {
                // Perspective aimed at target
                vector_Subtract(&ray.direction, &target, scene_GetEyePosition(scene));
                vector_Normalize(&ray.direction, &ray.direction);
            }
            
#ifdef DEBUG
            errmsg("Ray (%d, %d)\n", x, y);
            errmsg("Origin (%lf, %lf, %lf)\n", ray.origin.x, ray.origin.y, ray.origin.z);
            errmsg("Direction (%lf, %lf, %lf)\n", ray.direction.x, ray.direction.y, ray.direction.z);
#endif
            
            // Cast this ray
            if (raytrace_Cast(&collision, &ray, scene) != SUCCESS) {
                errmsg("Failed to cast ray (%d, %d)\n", x, y);
                return FAILURE;
            }
            
            // Determine color
            if (collision.how != COLLISION_NONE) {
                // Collided with the surface of the shape
                if (raytrace_Shade(&color, &collision, scene, INITIAL_REFRACTION, 0) != SUCCESS) {
                    errmsg("Shader failed\n");
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
                errmsg("Failed to set color at (%d, %d)\n", x, y);
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
