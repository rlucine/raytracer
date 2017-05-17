/**********************************************************//**
 * @file raytrace.c
 * @brief Implementation of raytracer implementation
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <stdlib.h>     // rand, RAND_MAX
#include <string.h>     // memcpy
#include <math.h>       // pow, tan, atan, INFINITY ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "geometry.h"   // PLANE, LINE
#include "scene.h"      // SCENE
#include "raytrace.h"   // SHADOW_THRESHOLD ...

// Debugging libraries
#include "debug.h"

/*============================================================*
 * Viewing plane
 *============================================================*/
typedef struct {
    VECTOR origin;   // Upper left corner
    VECTOR u;       // Unit vector u
    VECTOR v;       // Unit vector v
    float width;   // Real width of the viewing plane
    float height;  // Real height of the viewing plane
    VECTOR center;   // The center of the viewing plane
} VIEWPLANE;

/*============================================================*
 * Get the viewing plane
 *============================================================*/
static bool raytrace_GetView(VIEWPLANE *view, float view_distance, const SCENE *scene) {
    
    // Get the aspect ratio
    float aspect = (float)scene_GetWidth(scene) / (float)scene_GetHeight(scene);
    
    // Get the fields of view and plane dimensions
    float fov_vertical = M_PI * scene_GetFieldOfView(scene) / 180.0;
    float height = 2.0*tan(fov_vertical / 2.0);
    float width = height * aspect;
    
    // Get the u basis vector
    view->u = *scene_GetViewDirection(scene);
    vector_Cross(&view->u, scene_GetUpDirection(scene));
    vector_Normalize(&view->u);
    if (vector_IsZero(&view->u)) {
        eprintf("Null u vector (%f, %f, %f)\n", view->u.x, view->u.y, view->u.z);
        return false;
    }
    
    // Get the v basis vector
    view->v = view->u;
    vector_Cross(&view->v, scene_GetViewDirection(scene));
    vector_Normalize(&view->v);
    if (vector_IsZero(&view->v)) {
        eprintf("Null v vector (%f, %f, %f)\n", view->v.x, view->v.y, view->v.z);
        return false;
    }
    
    // Get the offset to the upper left
    VECTOR du, dv, distance;
    du = view->u;
    vector_Multiply(&du, width / -2.0);
    dv = view->v;
    vector_Multiply(&dv, height / 2.0);
    
    // Get the distance to the viewing plane
    distance = *scene_GetViewDirection(scene);
    vector_Normalize(&distance);
    vector_Multiply(&distance, view_distance);
    
    // Get the center point of the viewing plane
    view->center = *scene_GetEyePosition(scene);
    vector_Add(&view->center, &distance);
    
    // Get the upper left corner
    view->origin = *scene_GetEyePosition(scene);
    vector_Add(&view->origin, &distance);
    vector_Add(&view->origin, &du);
    vector_Add(&view->origin, &dv);
    
    // Scale basis vectors
    view->width = width;
    view->height = height;
    return true;
}

/*============================================================*
 * Cast one ray
 *============================================================*/
static bool raytrace_Cast(COLLISION *closest, const LINE *ray, const SCENE *scene) {
    
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
            eprintf("No shape with identifier %d\n", n);
            return false;
        }
        
        // Collide with this shape
        if (!shape_Collide(shape, ray, &current)) {
            eprintf("Collision with shape %d failed\n", n);
            return false;
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
        fprintf(stderr, "raytrace_Cast: Collided with %d at (%f, %f, %f)\n", who, closest->where.x, closest->where.y, closest->where.z);
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
    return true;
}

/*============================================================*
 * Recursive ray tracing (shadows)
 *============================================================*/
static float raytrace_Shadow(float *shadows, const COLLISION *collision, const LIGHT *light, const SCENE *scene) {
    
    // Set up ray pointing to light
    LINE ray;
    float distance;
    memcpy(&ray.origin, &collision->where, sizeof(VECTOR));
    if (!light_GetDirection(light, &collision->where, &ray.direction, &distance)) {
        eprintf("Invalid light\n");
        return false;
    }
    
    // Fire the shadow ray
    COLLISION shadow;
    if (!raytrace_Cast(&shadow, &ray, scene)) {
        eprintf("Failed to shoot shadow ray\n");
        return false;
    }
    
    // Check collisions
    if ((shadow.how != COLLISION_NONE) && (shadow.distance < distance) && (shadow.distance > COLLISION_THRESHOLD)) {
        // Something in between the light and us, and it isn't ourself!
        float alpha = 1.0 - shadow.material->opacity;
        
        // Check for all other collisions
        float rest;
        if (!raytrace_Shadow(&rest, &shadow, light, scene)) {
            eprintf("Failed to shadow all objects in the scene");
            return false;
        }
        
        // Take product of all alpha values in the way
        *shadows = alpha*rest;
    } else {
        *shadows = 1.0;
    }
    return true;
}

/*============================================================*
 * Recursive ray tracing (reflections)
 *============================================================*/

// Mutual recursion stuff
static bool raytrace_Shade(COLOR *, const COLLISION *, const SCENE *, float, int);

static bool raytrace_Reflection(COLOR *color, const COLLISION *collision, const SCENE *scene, float irefract, int depth) {
    
    // Stack overflow
    if (depth > RECURSION_DEPTH) {
        vector_Set(color, 0, 0, 0);
        return true;
    }
    
    // Get the normal which is in the same direction as the incident
    // vector so we do not have erroneous refraction / reflection
    VECTOR reflection_normal;
    if (vector_Dot(&collision->normal, &collision->incident) < 0) {
        reflection_normal = collision->normal;
        vector_Negate(&reflection_normal);
    } else {
        reflection_normal = collision->normal;
    }
    
    // Efficient computation of Fresnel reflectance
    const MATERIAL *material = collision->material;
    float cos_theta_i = vector_Dot(&reflection_normal, &collision->incident);
    assert(cos_theta_i >= 0);
    float pow1 = 1 - cos_theta_i;
    float pow2 = pow1 * pow1;
    float pow5 = pow2 * (pow1 * pow2);
    
    // Use the right value for fresnel_zero depending on opacity
    float fresnel_zero;
    if (fabs(material->opacity - 1.0) < DBL_EPSILON) {
        fresnel_zero = (material->refraction - 1.0) / (material->refraction + 1.0);
    } else {
        fresnel_zero = (material->refraction - irefract) / (material->refraction + irefract);
    }
    fresnel_zero *= fresnel_zero;
    
    // Compute the fresnel reflectance
    float fresnel = fresnel_zero + (1.0 - fresnel_zero)*pow5;
    assert(fresnel_zero <= fresnel);
    assert(fresnel <= 1);
    
    // Get the reflection ray's direction
    LINE reflection;
    reflection.direction = reflection_normal;
    vector_Multiply(&reflection.direction, 2.0*vector_Dot(&reflection_normal, &collision->incident));
    vector_Subtract(&reflection.direction, &collision->incident);
    
    // The reflection ray's position is the current collision
    reflection.origin = collision->where;

    // Shoot the reflection ray
    COLLISION reflection_collision;
    if (!raytrace_Cast(&reflection_collision, &reflection, scene)) {
        eprintf("Failed to shoot reflection ray\n");
        return false;
    }

    // Recursively shade the reflection color
    if (reflection_collision.how != COLLISION_NONE) {
        if (!raytrace_Shade(color, &reflection_collision, scene, irefract, depth+1)) {
            eprintf("Failed to shade the reflection ray\n");
            return false;
        }
        
        // Scale component with reflectivity
        vector_Multiply(color, fresnel);
        color_Clamp(color);
    } else {
        vector_Set(color, 0, 0, 0);
    }
    
    // Early exit if no transparency
    if (fabs(material->opacity - 1.0) < DBL_EPSILON) {
        return true;
    }
    
    // Set up the transparency ray direction
    LINE transparency;
    float ratio = irefract / collision->material->refraction;
    float tir_check = 1 - ((ratio*ratio)*(1 - cos_theta_i*cos_theta_i));
    if (tir_check < 0) {
        // Early exit on total internal refraction
        return true;
    }
    transparency.direction = reflection_normal;
    vector_Multiply(&transparency.direction, -sqrt(tir_check));
    VECTOR temp = reflection_normal;
    vector_Multiply(&temp, cos_theta_i);
    vector_Subtract(&temp, &collision->incident);
    vector_Multiply(&temp, ratio);
    vector_Add(&transparency.direction, &temp);
    
    // The transparency ray's location is the current collision
    transparency.origin = collision->where;
    
    // Shoot the transparency ray
    COLLISION transparency_collision;
    if (!raytrace_Cast(&transparency_collision, &transparency, scene)) {
        eprintf("Failed to shoot transparency ray\n");
        return false;
    }
    
    // Recursively shade transparency color
    COLOR transparency_color;
    if (!raytrace_Shade(&transparency_color, &transparency_collision, scene, material->refraction, depth+1)) {
        eprintf("Failed to shade the transparency ray\n");
        return false;
    }
        
    // Scale component
    float transparent_scale = (1 - fresnel)*(1 - material->opacity);
    assert(0 <= transparent_scale);
    assert(transparent_scale <= 1);
    vector_Multiply(&transparency_color, transparent_scale);
    color_Clamp(&transparency_color);
    vector_Add(color, &transparency_color);
    color_Clamp(color);
    return true;
}

/*============================================================*
 * Shader
 *============================================================*/
static bool raytrace_Shade(COLOR *color, const COLLISION *collision, const SCENE *scene, float irefract, int depth) {
    
    // Error check
    if (collision->how == COLLISION_NONE) {
        memcpy(color, &scene->background, sizeof(COLOR));
        return true;
    }
    
    // Get the diffuse color
    COLOR object_color;
    if (!shape_GetColorAt(collision, &object_color)) {
        eprintf("Failed to get object color\n");
        return false;
    }
    
    // Set the ambient color of the object
    const MATERIAL *material = collision->material;
    *color = object_color;
    vector_Multiply(color, material->ambient);
    
    // Shade color for all lights
    COLOR temp;
    float shadows;
    const LIGHT *light;
    for (int i = 0; i < scene_GetNumberOfLights(scene); i++) {
        // Check for shadows
        light = scene_GetLight(scene, i);
        
        // Get shadows and check float 
        if (!raytrace_Shadow(&shadows, collision, light, scene)) {
            eprintf("Failed to check shadows\n");
            return false;
        }
        if (shadows < SHADOW_THRESHOLD) {
            continue;
        }
        
        // Get shading for this light
        if (!light_BlinnPhongShade(light, collision, &temp)) {
            continue;
        }
        vector_Multiply(&temp, shadows);
        vector_Add(color, &temp);
    }
    color_Clamp(color);

    // Recursive ray tracing?
    if (depth < RECURSION_DEPTH) {
        // Determine any reflections
        COLOR reflection_color;
        if (!raytrace_Reflection(&reflection_color, collision, scene, irefract, depth)) {
            eprintf("Failed to get reflection color\n");
            return false;
        }
        
        // Incorporate the reflected color into the result
        vector_Add(color, &reflection_color);
        color_Clamp(color);
    }

    return true;
}

/*============================================================*
 * Generate an image
 *============================================================*/
bool raytrace_Render(IMAGE *image, const SCENE *scene) {
    
    // Get the scene view
    VIEWPLANE view;
    float distance = VIEW_DISTANCE;
    if (scene->flags & PROJECT_PARALLEL) {
        distance = 0.0;
    }
    if (!raytrace_GetView(&view, distance, scene)) {
        eprintf("Failed to generate viewing plane\n");
        return false;
    }
    
#ifdef DEBUG
    eprintf("Viewing plane origin is (%f, %f, %f)\n", view.origin.x, view.origin.y, view.origin.z);
    eprintf("Viewing plane u is (%f, %f, %f)\n", view.u.x, view.u.y, view.u.z);
    eprintf("Viewing plane v is (%f, %f, %f)\n", view.v.x, view.v.y, view.v.z);
    eprintf("Viewing plane size is %f by %f\n", view.width, view.height);
#endif
    
    // Get the image output
    if (!image_Create(image, scene_GetWidth(scene), scene_GetHeight(scene))) {
        eprintf("Failed to create output image\n");
        return false;
    }
    
    // Image pixel size
    int width = image_GetWidth(image);
    int height = image_GetHeight(image);
    
    // This is the ray to shoot
    LINE ray;
    ray.origin = *scene_GetEyePosition(scene);
    
    // What is the ray target
    VECTOR target = view.origin;
    
    // Establish the step
    VECTOR dx = view.u;
    VECTOR dy = view.v;
    vector_Multiply(&dx, view.width / (scene_GetWidth(scene) - 1));
    
    // Negate v as we are moving from upper left to lower right
    vector_Multiply(&dy, -view.height / (scene_GetHeight(scene) - 1));
    
    // We buffer the step in Y separately because of floating point
    // error if we mathematically go backwards
    VECTOR ystep;
    vector_Set(&ystep, 0, 0, 0);
    
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
                ray.origin = target;
                ray.direction = *scene_GetViewDirection(scene);
                vector_Normalize(&ray.direction);
            } else {
                // Perspective aimed at target
                ray.direction = target;
                vector_Subtract(&ray.direction, scene_GetEyePosition(scene));
                vector_Normalize(&ray.direction);
            }
            
#ifdef DEBUG
            eprintf("Ray (%d, %d)\n", x, y);
            eprintf("Origin (%f, %f, %f)\n", ray.origin.x, ray.origin.y, ray.origin.z);
            eprintf("Direction (%f, %f, %f)\n", ray.direction.x, ray.direction.y, ray.direction.z);
#endif
            
            // Cast this ray
            if (!raytrace_Cast(&collision, &ray, scene)) {
                eprintf("Failed to cast ray (%d, %d)\n", x, y);
                return false;
            }
            
            // Determine color
            if (collision.how != COLLISION_NONE) {
                // Collided with the surface of the shape
                if (!raytrace_Shade(&color, &collision, scene, INITIAL_REFRACTION, 0)) {
                    eprintf("Shader failed\n");
                    return false;
                }
                
                // Get the RGB color
                color_ToRgb(&rgb, &color);
                
            } else {
                // No shapes, no collision, or inside a shape
                color_ToRgb(&rgb, scene_GetBackgroundColor(scene));
            }
            
            // Put the color
            if (!image_SetPixel(image, x, y, &rgb)) {
                eprintf("Failed to set color at (%d, %d)\n", x, y);
                return false;
            }
#ifdef DEBUG
            const RGB *what = image_GetPixel(image, x, y);
            assert(what->r == rgb.r);
            assert(what->g == rgb.g);
            assert(what->b == rgb.b);
#endif
            
            // Step forward in x
            vector_Add(&target, &dx);
            x++;
        }
        
        // Step forward in y
        vector_Add(&ystep, &dy);
        target = view.origin;
        vector_Add(&target, &ystep);
        y++;
    }
    return true;
}

/*============================================================*/
