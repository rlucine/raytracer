/**********************************************************//**
 * @file light.h
 * @brief Implementation of lighting
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <math.h>       // pow, INFINITY, M_PI
#include <stdio.h>      // stderr, fprintf

// This project
#include "light.h"      // LIGHT
#include "color.h"      // COLOR, color_Clamp ...
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE

// Debugging libraries
#include "debug.h"

/*============================================================*
 * Creation
 *============================================================*/
void light_CreatePoint(LIGHT *light, const VECTOR *where, const COLOR *color) {
    // Creates a point light
    light->where = *where;
    light->color = *color;
    light->type = LIGHT_POINT;
    light->angle = 0.0;
    vector_Set(&light->direction, 0, 0, 0);
}

void light_CreateDirected(LIGHT *light, const VECTOR *direction, const COLOR *color) {
    // Creates a directional light
    light->direction = *direction;
    light->color = *color;
    light->type = LIGHT_DIRECTED;
    light->angle = 0.0;
    vector_Set(&light->where, 0, 0, 0);
}

void light_CreateSpotlight(LIGHT *light, const VECTOR *where, const VECTOR *direction, float angle, const COLOR *color) {
    // Creates a spotlight
    light->where = *where;
    light->direction = *direction;
    light->color = *color;
    light->type = LIGHT_SPOT;
    light->angle = angle;
}

/*============================================================*
 * Direction
 *============================================================*/
bool light_GetDirection(const LIGHT *light, const VECTOR *where, VECTOR *output, float *distance) {
    switch (light->type) {
    case LIGHT_SPOT:
    case LIGHT_POINT:
        // Get direction to origin of light
        *output = light->where;
        vector_Subtract(output, where);
        if (distance) {
            *distance = vector_Length(output);
        }
        break;
    
    case LIGHT_DIRECTED:
        // The light is just a direction
        *output = light->direction;
        vector_Negate(output);
        if (distance) {
            *distance = INFINITY;
        }
        break;
    
    case LIGHT_NONE:
    default:
        eprintf("Light type undefined\n");
        return false;
    }
    
    // Return only unit vectors
    vector_Normalize(output);
    return true;
}

/*============================================================*
 * Light shader
 *============================================================*/
bool light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, COLOR *color) {
    // Get the vector pointing from the collision to the light
    VECTOR to_light;
    if (!light_GetDirection(light, &collision->where, &to_light, NULL)) {
        eprintf("Cannot get direction to light\n");
        return false;
    }
    
    // Check spotlight radius
    if (light->type == LIGHT_SPOT) {
        float radians = light->angle * M_PI / 180;
        VECTOR temp = to_light;
        vector_Negate(&temp);
        if (vector_Angle(&temp, &light->direction) > radians) {
            // Outside the spotlight!
            return false;
        }
    }
    
    // Get the view vector
    VECTOR view = collision->incident;
    
    // Get the halfway vector
    VECTOR halfway = to_light;
    vector_Add(&halfway, &view);
    vector_Normalize(&halfway);
    
    // Get the diffuse color
    COLOR object_color;
    if (!shape_GetColorAt(collision, &object_color)) {
        eprintf("Failed to get object color\n");
        return false;
    }
    
    // Don't factor in ambient color at all
    VECTOR temp;
    const MATERIAL *material = collision->material;
    vector_Set(color, 0, 0, 0);
    
    // Diffuse color components - clamp to positive
    float diffuse = vector_Dot(&collision->normal, &to_light) * material->diffuse;
    if (diffuse > 0.0) {
        temp = object_color;
        vector_Multiply(&temp, diffuse);
        vector_Add(color, &temp);
    }
    
    // Specular components
    float dot = vector_Dot(&halfway, &collision->normal);
    float specular = pow(dot, material->exponent) * material->specular;
    if (specular > 0.0) {
        temp = material->highlight;
        vector_Multiply(&temp, specular);
        vector_Add(color, &temp);
    }
    
#ifdef DEBUG
    eprintf("Normal is (%f, %f, %f)\n", collision->normal.x, collision->normal.y, collision->normal.z);
    eprintf("View is (%f, %f, %f)\n", view.x, view.y, view.z);
    eprintf("Halfway is (%f, %f, %f)\n", halfway.x, halfway.y, halfway.z);
    eprintf("Light is (%f, %f, %f)\n", to_light.x, to_light.y, to_light.z);
    eprintf("Diffuse coefficient is %f\n", diffuse);
    eprintf("Specular coefficient is %f\n", specular);
    eprintf("Dot is %f and dot^%d is %f\n", dot, material->exponent, pow(dot, material->exponent));
#endif
    
    // Scale by light's own color
    color_Clamp(color);
    color->x *= light->color.x;
    color->y *= light->color.y;
    color->z *= light->color.z;
    
    // Done with color
    return true;
}

/*============================================================*/
