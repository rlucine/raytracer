/**********************************************************//**
 * @file shape.c
 * @brief Implementation of 3D shape geometry
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // free
#include <math.h>       // sqrt, asin, M_PI, fabs ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "rgb.h"
#include "vector.h"
#include "shape.h"

#define SUCCESS 0
#define FAILURE -1

/*============================================================*
 * Accessors
 *============================================================*/
const MATERIAL *shape_GetMaterial(const SHAPE *shape) {
    return &shape->material;
}

const POINT *sphere_GetCenter(const SPHERE *sphere) {
    return &sphere->center;
}

const double sphere_GetRadius(const SPHERE *sphere) {
    return sphere->radius;
}

const POINT *ellipsoid_GetCenter(const ELLIPSOID *ellipsoid) {
    return &ellipsoid->center;
}

const VECTOR *ellipsoid_GetDimension(const ELLIPSOID *ellipsoid) {
    return &ellipsoid->dimension;
}

int shape_IsShape(const SHAPE *shape, SHAPE_TYPE what) {
    return shape->shape == what;
}

/*============================================================*
 * Sphere geometry
 *============================================================*/
int sphere_Collide(const SPHERE *sphere, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
#ifdef DEBUG
        fprintf(stderr, "sphere_Collide failed: direction is the null vector\n");
#endif
        return FAILURE;
    }
    if (sphere_GetRadius(sphere) <= 0.0) {
#ifdef DEBUG
        fprintf(stderr, "sphere_Collide failed: Sphere radius nonpositive\n");
#endif
        return FAILURE;
    }
    
    // Check if the point resides in the sphere
    register double radius = sphere_GetRadius(sphere);
    register double dis_center;
    VECTOR vec_center;
    vector_Subtract(&vec_center, &ray->origin, sphere_GetCenter(sphere));
    dis_center = vector_Magnitude(&vec_center);
    if (dis_center <= radius) {
        // The origin is inside of the sphere already
        result->how = COLLISION_INSIDE;
        return SUCCESS;
    }
    
    // Theta is the angle between the view direction and the direction to the sphere
    double theta = vector_Angle(&ray->direction, &vec_center);
    
    // Get the distance from the sphere's center to the line
    // of view from the origin.
    double dis_line = dis_center * sin(theta);
    double dis_temp = dis_center * cos(theta);
    if (dis_line > radius || dis_temp < 0.0) {
        // The line does not intersect the sphere
        result->how = COLLISION_NONE;
        return SUCCESS;
    }
    
    // Phi is the angle we need to rotate to find points on the
    // surface of the sphere from where we determined the shortest
    // distance to the line was.
    double phi = (M_PI / 2) - asin(dis_line / radius);
    
    // Points of intersection at dis_temp plus or minus dis_delta
    double dis_delta = radius * sin(phi);
    double dis_intersect;
    if (dis_delta > 0) {
        dis_intersect = dis_temp - dis_delta;
    } else {
        dis_intersect = dis_temp + dis_delta;
    }
    
    // Get the nearest point of intersection
    result->how = COLLISION_SURFACE;
    result->distance = dis_intersect;
    vector_Multiply(&result->where, &ray->direction, dis_intersect / vector_Magnitude(&ray->direction));
    return SUCCESS;
}

/*============================================================*
 * Ellipsoid geometry
 *============================================================*/
int ellipsoid_Collide(const ELLIPSOID *ellipsoid, const LINE *rar, COLLISION *result) {
    // TODO
    result->how = COLLISION_NONE;
    return SUCCESS;
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
    
#ifdef DEBUG
    fprintf(stderr, "shape_Collide failed: No collision for shape type %d\n", shape->shape);
#endif
    return FAILURE;
}

/*============================================================*/