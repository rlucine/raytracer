/**********************************************************//**
 * @file light.h
 * @brief Implementation of lighting
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <math.h>   // pow, INFINITY
#include <stdio.h>  // stderr, fprintf

// This project
#include "macro.h"
#include "light.h"
#include "color.h"
#include "vector.h"
#include "shape.h"

/*============================================================*
 * Direction
 *============================================================*/
int light_GetDirection(const LIGHT *light, const POINT *where, VECTOR *output, double *distance) {
    switch (light->type) {
    case LIGHT_POINT:
    case LIGHT_SPOT:
        vector_Subtract(output, &light->where, where);
        *distance = vector_Magnitude(output);
        break;
    
    case LIGHT_DIRECTED:
        vector_Copy(output, &light->direction);
        *distance = INFINITY;
        break;
    
    case LIGHT_NONE:
    default:
        return FAILURE;
    }
    return SUCCESS;
}

/*============================================================*
 * Light shader
 *============================================================*/
int light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, const POINT *eye, COLOR *color) {
    // Shade the given collision with the light
    
    // Get the vector pointing from the collision to the light
    VECTOR to_light;
    switch (light->type) {
    case LIGHT_POINT:
        // Get the vector to the light
        vector_Subtract(&to_light, &light->where, &collision->where);
        break;
        
    case LIGHT_DIRECTED:
        // The vector to the light is trivial, just negate direction
        vector_Negate(&to_light, &light->direction);
        break;
        
    case LIGHT_SPOT:
        // Let's see if the spot light actually hits
        vector_Subtract(&to_light, &light->where, &collision->where);
        if (vector_Angle(&to_light, &light->direction) > light->angle) {
            // Outside the spotlight!
            return FAILURE;
        }
        
    case LIGHT_NONE:
    default:
        // No lighting
        return FAILURE;
    }
    vector_Normalize(&to_light, &to_light);
    
    // Get the halfway vector
    VECTOR halfway;
    vector_Subtract(&halfway, &collision->where, eye);  // Direction to eye
    vector_Add(&halfway, &to_light, &halfway);          // Add direction to light
    vector_Normalize(&halfway, &halfway);               // Normalize
    
    // Don't factor in ambient color at all
    VECTOR temp;
    const MATERIAL *material = collision->material;
    color->x = color->y = color->z = 0.0;
    
    // Diffuse color components - clamp to positive
    double diffuse = vector_Dot(&collision->normal, &to_light) * material->diffuse;
    if (diffuse > 0.0) {
        vector_Multiply(&temp, &material->color, diffuse);
        vector_Add(color, color, &temp);
    }
    
    // Specular components
    double specular = pow(vector_Dot(&halfway, &to_light), material->exponent) * material->specular;
    if (specular > 0.0) {
        vector_Multiply(&temp, &material->highlight, specular);
        vector_Add(color, color, &temp);
    }
    
#ifdef DEBUG
    fprintf(stderr, "light_BlinnPhongShade: Normal is (%lf, %lf, %lf)\n", collision->normal.x, collision->normal.y, collision->normal.z);
    fprintf(stderr, "light_BlinnPhongShade: Halfway is (%lf, %lf, %lf)\n", halfway.x, halfway.y, halfway.z);
    fprintf(stderr, "light_BlinnPhongShade: Light is (%lf, %lf, %lf)\n", to_light.x, to_light.y, to_light.z);
    fprintf(stderr, "light_BlinnPhongShade: Diffuse coefficient is %lf\n", diffuse);
    fprintf(stderr, "light_BlinnPhongShade: Specular coefficient is %lf\n", specular);
#endif
    
    // Scale by light's own color
    color_Clamp(color);
    color->x *= light->color.x;
    color->y *= light->color.y;
    color->z *= light->color.z;
    
    // Done with color
    return SUCCESS;
}

/*============================================================*/
