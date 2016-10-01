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
        // Get direction to origin of light
        vector_Subtract(output, &light->where, where);
        
        if (distance) {
            *distance = vector_Magnitude(output);
        }
        break;
    
    case LIGHT_DIRECTED:
        // The light is just a direction
        vector_Negate(output, &light->direction);
        
        if (distance) {
            *distance = INFINITY;
        }
        break;
    
    case LIGHT_NONE:
    default:
#ifdef VERBOSE
        fprintf(stderr, "light_GetDirection failed: Light type undefined\n");
#endif
        return FAILURE;
    }
    
    // Return only unit vectors
    vector_Normalize(output, output);
    return SUCCESS;
}

/*============================================================*
 * Light shader
 *============================================================*/
int light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, const POINT *eye, COLOR *color) {
    // Shade the given collision with the light
    
    // Get the vector pointing from the collision to the light
    VECTOR to_light;
    if (light_GetDirection(light, &collision->where, &to_light, NULL) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "light_BlinnPhongShade failed: Cannot get direction to light\n");
#endif
        return FAILURE;
    }
    
    // Check spotlight radius
    if (light->type == LIGHT_SPOT) {
        if (vector_Angle(&to_light, &light->direction) > light->angle) {
            // Outside the spotlight!
            return FAILURE;
        }
    }
    
    // Get the view vector
    VECTOR view;
    vector_Subtract(&view, eye, &collision->where);
    vector_Normalize(&view, &view);
    
    // Get the halfway vector
    VECTOR halfway;
    vector_Add(&halfway, &to_light, &view);
    vector_Normalize(&halfway, &halfway);
    
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
    double dot = vector_Dot(&halfway, &collision->normal);
    double specular = pow(dot, material->exponent) * material->specular;
    if (specular > 0.0) {
        vector_Multiply(&temp, &material->highlight, specular);
        vector_Add(color, color, &temp);
    }
    
#ifdef DEBUG
    fprintf(stderr, "light_BlinnPhongShade: Normal is (%lf, %lf, %lf)\n", collision->normal.x, collision->normal.y, collision->normal.z);
    fprintf(stderr, "light_BlinnPhongShade: Eye is (%lf, %lf, %lf)\n", eye->x, eye->y, eye->z);
    fprintf(stderr, "light_BlinnPhongShade: View is (%lf, %lf, %lf)\n", view.x, view.y, view.z);
    fprintf(stderr, "light_BlinnPhongShade: Halfway is (%lf, %lf, %lf)\n", halfway.x, halfway.y, halfway.z);
    fprintf(stderr, "light_BlinnPhongShade: Light is (%lf, %lf, %lf)\n", to_light.x, to_light.y, to_light.z);
    fprintf(stderr, "light_BlinnPhongShade: Diffuse coefficient is %lf\n", diffuse);
    fprintf(stderr, "light_BlinnPhongShade: Specular coefficient is %lf\n", specular);
    fprintf(stderr, "light_BlinnPhongShade: Dot is %lf and dot^%d is %lf\n", dot, material->exponent, pow(dot, material->exponent));
    
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
