/**********************************************************//**
 * @file light.h
 * @brief Implementation of lighting
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <math.h>   // pow, INFINITY, M_PI
#include <stdio.h>  // stderr, fprintf

// This project
#include "macro.h"  // SUCCESS, FAILURE
#include "light.h"  // LIGHT
#include "color.h"  // COLOR, color_Clamp ...
#include "vector.h" // VECTOR
#include "shape.h"  // SHAPE

// Debugging libraries
#include "debug.h"

/*============================================================*
 * Creation
 *============================================================*/
void light_CreatePoint(LIGHT *light, const POINT *where, const COLOR *color) {
    // Creates a point light
    vector_Copy(&light->where, where);
    vector_Copy(&light->color, color);
    light->type = LIGHT_POINT;
    light->angle = 0.0;
    vector_Set(&light->direction, 0, 0, 0);
}

void light_CreateDirected(LIGHT *light, const VECTOR *direction, const COLOR *color) {
    // Creates a directional light
    vector_Copy(&light->direction, direction);
    vector_Copy(&light->color, color);
    light->type = LIGHT_DIRECTED;
    light->angle = 0.0;
    vector_Set(&light->where, 0, 0, 0);
}

void light_CreateSpotlight(LIGHT *light, const POINT *where, const VECTOR *direction, double angle, const COLOR *color) {
    // Creates a spotlight
    vector_Copy(&light->where, where);
    vector_Copy(&light->direction, direction);
    vector_Copy(&light->color, color);
    light->type = LIGHT_SPOT;
    light->angle = angle;
}

/*============================================================*
 * Direction
 *============================================================*/
int light_GetDirection(const LIGHT *light, const POINT *where, VECTOR *output, double *distance) {
    switch (light->type) {
    case LIGHT_SPOT:
    case LIGHT_POINT:
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
        errmsg("Light type undefined\n");
        return FAILURE;
    }
    
    // Return only unit vectors
    vector_Normalize(output, output);
    return SUCCESS;
}

/*============================================================*
 * Light shader
 *============================================================*/
int light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, COLOR *color) {
    // Shade the given collision with the light
    
    // Get the vector pointing from the collision to the light
    VECTOR to_light;
    if (light_GetDirection(light, &collision->where, &to_light, NULL) != SUCCESS) {
        errmsg("Cannot get direction to light\n");
        return FAILURE;
    }
    
    // Check spotlight radius
    if (light->type == LIGHT_SPOT) {
        double radians = light->angle * M_PI / 180;
        VECTOR temp;
        vector_Negate(&temp, &to_light);
        if (vector_Angle(&temp, &light->direction) > radians) {
            // Outside the spotlight!
            return FAILURE;
        }
    }
    
    // Get the view vector
    VECTOR view;
    vector_Copy(&view, &collision->incident);
    
    // Get the halfway vector
    VECTOR halfway;
    vector_Add(&halfway, &to_light, &view);
    vector_Normalize(&halfway, &halfway);
    
    // Get the diffuse color
    COLOR object_color;
    if (shape_GetColorAt(collision, &object_color) != SUCCESS) {
        errmsg("Failed to get object color\n");
        return FAILURE;
    }
    
    // Don't factor in ambient color at all
    VECTOR temp;
    const MATERIAL *material = collision->material;
    vector_Set(color, 0, 0, 0);
    
    // Diffuse color components - clamp to positive
    double diffuse = vector_Dot(&collision->normal, &to_light) * material->diffuse;
    if (diffuse > 0.0) {
        vector_Multiply(&temp, &object_color, diffuse);
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
    errmsg("Normal is (%lf, %lf, %lf)\n", collision->normal.x, collision->normal.y, collision->normal.z);
    errmsg("View is (%lf, %lf, %lf)\n", view.x, view.y, view.z);
    errmsg("Halfway is (%lf, %lf, %lf)\n", halfway.x, halfway.y, halfway.z);
    errmsg("Light is (%lf, %lf, %lf)\n", to_light.x, to_light.y, to_light.z);
    errmsg("Diffuse coefficient is %lf\n", diffuse);
    errmsg("Specular coefficient is %lf\n", specular);
    errmsg("Dot is %lf and dot^%d is %lf\n", dot, material->exponent, pow(dot, material->exponent));
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
