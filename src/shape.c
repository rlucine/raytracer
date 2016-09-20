/**********************************************************//**
 * @file shape.c
 * @brief Implementation of 3D shape geometry
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // free
#include <math.h>       // sqrt, asin, M_PI, fabs, sqrt ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "image.h"
#include "vector.h"
#include "shape.h"

/*============================================================*
 * Shape accessors
 *============================================================*/
const MATERIAL *shape_GetMaterial(const SHAPE *shape) {
    return &shape->material;
}

const SPHERE *shape_GetSphere(const SHAPE *shape) {
    if (shape->shape == SHAPE_SPHERE) {
        return (SPHERE *)shape->data;
    }
    return NULL;
}

const ELLIPSOID *shape_GetEllipsoid(const SHAPE *shape) {
    if (shape->shape == SHAPE_ELLIPSOID) {
        return (ELLIPSOID *)shape->data;
    }
    return NULL;
}

SHAPE_TYPE shape_GetGeometry(const SHAPE *shape) {
    return shape->shape;
}

/*============================================================*
 * Sphere accessors
 *============================================================*/
const POINT *sphere_GetCenter(const SPHERE *sphere) {
    return &sphere->center;
}

double sphere_GetRadius(const SPHERE *sphere) {
    return sphere->radius;
}

/*============================================================*
 * Ellipsoid accessors
 *============================================================*/
const POINT *ellipsoid_GetCenter(const ELLIPSOID *ellipsoid) {
    return &ellipsoid->center;
}

const VECTOR *ellipsoid_GetDimension(const ELLIPSOID *ellipsoid) {
    return &ellipsoid->dimension;
}

/*============================================================*
 * Sphere geometry
 *============================================================*/
int sphere_Collide(const SPHERE *sphere, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
#ifdef VERBOSE
        fprintf(stderr, "sphere_Collide failed: direction is the null vector\n");
#endif
        return FAILURE;
    }
    if (sphere_GetRadius(sphere) <= 0.0) {
#ifdef VERBOSE
        fprintf(stderr, "sphere_Collide failed: Sphere radius nonpositive\n");
#endif
        return FAILURE;
    }

    // Solve for sphere intersection with line
    VECTOR unit_direction;
    vector_Unit(&unit_direction, &ray->direction);
    double a = 1.0;
    VECTOR dis_center;
    vector_Subtract(&dis_center, &ray->origin, &sphere->center);
    double b = 2.0*vector_Dot(&unit_direction, &dis_center);
    double c = vector_Dot(&dis_center, &dis_center) - (sphere->radius * sphere->radius);

    // Is it inside the sphere
    if (vector_Magnitude(&dis_center) <= sphere->radius) {
        result->how = COLLISION_INSIDE;
        result->distance=  0.0;
        vector_Copy(&result->where, &ray->origin);
        return SUCCESS;
    }

    // Solve the quadratic att + bt + c = 0
    double discriminant = b*b - 4.0*a*c;
    if (discriminant < 0.0) {
        // No solutions - missed the sphere
        result->how = COLLISION_NONE;
        return SUCCESS;
    }

    // Two or one solution
    double t1, t2;
    t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    t2 = (-b - sqrt(discriminant)) / (2.0 * a);

    // Determine closest collision
    double tclosest = 0.0;
    if (t1 < t2 && t1 >= 0) {
        tclosest = t1;
    } else if (t2 >= 0) {
        tclosest = t2;
    } else {
        // The sphere is behind the viewer - miss!
        result->how = COLLISION_NONE;
        return SUCCESS;
    }

    // Get location of closest collision
    result->how = COLLISION_SURFACE;
    result->distance = tclosest;
    vector_Multiply(&result->where, &unit_direction, tclosest);
    return SUCCESS;
}

/*============================================================*
 * Ellipsoid geometry
 *============================================================*/
int ellipsoid_Collide(const ELLIPSOID *ellipsoid, const LINE *ray, COLLISION *result) {
    // TODO
    return FAILURE;
}

/*============================================================*
 * Generalized geometry
 *============================================================*/
int shape_Collide(const SHAPE *shape, const LINE *ray, COLLISION *result) {
    
    // Map to specific collision checking functions
    switch (shape->shape) {
    case SHAPE_SPHERE:
        return sphere_Collide((SPHERE *)shape->data, ray, result);
        
    case SHAPE_ELLIPSOID:
        return ellipsoid_Collide((ELLIPSOID *)shape->data, ray, result);
    
    case SHAPE_NONE:
    default:
        break;
    }
    
#ifdef VERBOSE
    fprintf(stderr, "shape_Collide failed: No collision for shape type %d\n", shape->shape);
#endif
    return FAILURE;
}

/*============================================================*/
