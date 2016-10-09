/**********************************************************//**
 * @file shape.c
 * @brief Implementation of 3D shape geometry
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // free
#include <string.h>     // memcpy
#include <math.h>       // sqrt, asin, M_PI, fabs, sqrt ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR, PLANE ...
#include "shape.h"      // SHAPE
#include "mesh.h"       // MESH, FACE ...

// Debugging modules
#ifdef DEBUG
#include "tracemalloc.h"
#endif

/*============================================================*
 * Shape creation
 *============================================================*/
int shape_CreateSphere(SHAPE *shape, const SPHERE *sphere, const MATERIAL *material) {
    // Initialize a new sphere
    shape->shape = SHAPE_SPHERE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(SPHERE));
    if (!shape->data) {
#ifdef VERBOSE
        fprintf(stderr, "shape_CreateSphere failed: Out of memory\n");
#endif
        return FAILURE;
    }
    memcpy(shape->data, sphere, sizeof(SPHERE));
    
    // Copy material into shape
    shape->material = material;
    return SUCCESS;
}

int shape_CreateEllipsoid(SHAPE *shape, const ELLIPSOID *ellipsoid, const MATERIAL *material) {
    // Initialize a new ellipsoid
    shape->shape = SHAPE_ELLIPSOID;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(ELLIPSOID));
    if (!shape->data) {
#ifdef VERBOSE
        fprintf(stderr, "shape_CreateEllipsoid failed: Out of memory\n");
#endif
        return FAILURE;
    }
    memcpy(shape->data, ellipsoid, sizeof(ELLIPSOID));
    
    // Copy material into shape
    shape->material = material;
    return SUCCESS;
}

int shape_CreatePlane(SHAPE *shape, const PLANE *plane, const MATERIAL *material) {
    // Initialize a new infinite plane
    shape->shape = SHAPE_PLANE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(PLANE));
    if (!shape->data) {
#ifdef VERBOSE
        fprintf(stderr, "shape_CreatePlane failed: Out of memory\n");
#endif
        return FAILURE;
    }
    memcpy(shape->data, plane, sizeof(PLANE));
    
    // Copy material into shape
    shape->material = material;
    return SUCCESS;
}

int shape_CreateFace(SHAPE *shape, const FACE *face, const MATERIAL *material) {
    // Initialize a new triangle face
    shape->shape = SHAPE_FACE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(FACE));
    if (!shape->data) {
#ifdef VERBOSE
        fprintf(stderr, "shape_CreateFace failed: Out of memory\n");
#endif
        return FAILURE;
    }
    memcpy(shape->data, face, sizeof(FACE));
    
    // Copy material into shape
    shape->material = material;
    return SUCCESS;
}

/*============================================================*
 * Shape destruction
 *============================================================*/
void shape_Destroy(SHAPE *shape) {
    if (shape->data) {
        free(shape->data);
        shape->data = NULL;
    }
    return;
}

/*============================================================*
 * Shape accessors
 *============================================================*/
const MATERIAL *shape_GetMaterial(const SHAPE *shape) {
    return shape->material;
}

void shape_SetMaterial(SHAPE *shape, const MATERIAL *material) {
    shape->material = material;
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

const PLANE *shape_GetPlane(const SHAPE *shape) {
    if (shape->shape == SHAPE_PLANE) {
        return (PLANE *)shape->data;
    }
    return NULL;
}

const FACE *shape_GetFace(const SHAPE *shape) {
    if (shape->shape == SHAPE_FACE) {
        return (FACE *)shape->data;
    }
    return NULL;
}

/*============================================================*
 * Sphere geometry
 *============================================================*/
static int sphere_Collide(const SPHERE *sphere, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
#ifdef VERBOSE
        fprintf(stderr, "sphere_Collide failed: direction is the null vector\n");
#endif
        return FAILURE;
    }
    if (sphere->radius <= 0.0) {
#ifdef VERBOSE
        fprintf(stderr, "sphere_Collide failed: Sphere radius nonpositive\n");
#endif
        return FAILURE;
    }

    // Solve for sphere intersection with line
    VECTOR unit_direction;
    vector_Normalize(&unit_direction, &ray->direction);
    double a = 1.0;
    VECTOR dis_center;
    vector_Subtract(&dis_center, &ray->origin, &sphere->center);
    double b = 2.0*vector_Dot(&unit_direction, &dis_center);
    double c = vector_Dot(&dis_center, &dis_center) - (sphere->radius * sphere->radius);

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
    if (vector_Magnitude(&dis_center) <= sphere->radius) {
        result->how = COLLISION_INSIDE;
    } else {
        result->how = COLLISION_SURFACE;
    }
    result->distance = tclosest;
    vector_Multiply(&result->where, &unit_direction, tclosest);
    vector_Add(&result->where, &result->where, &ray->origin);
    
    // Get normal vector at collision
    vector_Subtract(&result->normal, &result->where, &sphere->center);
    vector_Normalize(&result->normal, &result->normal);
    
    // Done!
    return SUCCESS;
}

/*============================================================*
 * Ellipsoid geometry
 *============================================================*/
static int ellipsoid_Collide(const ELLIPSOID *ellipsoid, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
#ifdef VERBOSE
        fprintf(stderr, "ellipsoid_Collide failed: direction is the null vector\n");
#endif
        return FAILURE;
    }
    
    const VECTOR *dimension = &ellipsoid->dimension;
    if (dimension->x <= 0.0 || dimension->y <= 0.0 || dimension->z <= 0.0) {
#ifdef VERBOSE
        fprintf(stderr, "ellipsoid_Collide failed: Ellipsoid with negative dimension\n");
#endif
        return FAILURE;
    }

    // Get the unit direction of the ray
    VECTOR unit;
    vector_Normalize(&unit, &ray->direction);
    
    // Get the distance to the center of the ellipsoid
    VECTOR center;
    vector_Subtract(&center, &ray->origin, &ellipsoid->center);
    
    double a = 0.0;
    a += unit.x*unit.x / (dimension->x * dimension->x);
    a += unit.y*unit.y / (dimension->y * dimension->y);
    a += unit.z*unit.z / (dimension->z * dimension->z);
    
    double b = 0.0;
    b += 2*(center.x * unit.x / (dimension->x * dimension->x));
    b += 2*(center.y * unit.y / (dimension->y * dimension->y));
    b += 2*(center.z * unit.z / (dimension->z * dimension->z));
    
    double c = -1.0;
    c += center.x*center.x / (dimension->x * dimension->x);
    c += center.y*center.y / (dimension->y * dimension->y);
    c += center.z*center.z / (dimension->z * dimension->z);
    
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
    // This absorbs case when we collide immediately
    result->how = COLLISION_SURFACE;
    result->distance = tclosest;
    vector_Multiply(&result->where, &unit, tclosest);
    vector_Add(&result->where, &result->where, &ray->origin);
    
    // Get the normal vector at the collision site
    vector_Subtract(&result->normal, &result->where, &ellipsoid->center);
    result->normal.x *= 2.0 / (dimension->x * dimension->x);
    result->normal.y *= 2.0 / (dimension->y * dimension->y);
    result->normal.z *= 2.0 / (dimension->z * dimension->z);
    vector_Normalize(&result->normal, &result->normal);
    
    // Done
    return SUCCESS;
}

/*============================================================*
 * Plane geometry
 *============================================================*/
static int plane_Collide(const PLANE *plane, const LINE *ray, COLLISION *result) {
    
    // Collide the ray with the plane
    if (vector_IsZero(&ray->direction)) {
#ifdef VERBOSE
        fprintf(stderr, "plane_Collide failed: direction is the null vector\n");
#endif
        return FAILURE;
    }
    
    // Get the unit direction of the ray
    VECTOR unit;
    vector_Normalize(&unit, &ray->direction);
    
    // Get the plane's normal vector
    VECTOR normal;
    vector_Cross(&normal, &plane->u, &plane->v);
    vector_Normalize(&normal, &normal);
    
    // Determine where the ray and plane intersect
    VECTOR offset;
    vector_Subtract(&offset, &plane->origin, &ray->origin);
    double tclosest, numerator, denominator = vector_Dot(&normal, &unit);
    if (fabs(denominator) <= DBL_EPSILON) {
        if (fabs(vector_Dot(&offset, &normal)) <= DBL_EPSILON) {
            // Inside plane
            numerator = 0.0;
            tclosest = 0.0;
        } else {
            // Paralell to plane - no collision
            result->how = COLLISION_NONE;
            return SUCCESS;
        }
    } else {
        numerator = vector_Dot(&normal, &offset);
        tclosest = numerator / denominator;
    }
    
    // Determine collision location
    if (tclosest < 0.0) {
        // The plane is behind the viewer - miss!
        result->how = COLLISION_NONE;
        return SUCCESS;
    } else if (tclosest == 0.0) {
        // Rarely happens
        result->how = COLLISION_INSIDE;
    } else {
        result->how = COLLISION_SURFACE;
    }
    
    // Determine where the collision is
    result->distance = tclosest;
    vector_Multiply(&result->where, &unit, tclosest);
    vector_Add(&result->where, &result->where, &ray->origin);
    
    // Determine normal at collision site
    vector_Copy(&result->normal, &normal);
    return SUCCESS;
}

/*============================================================*
 * Face geometry
 *============================================================*/
static int face_Collide(const FACE *face, const LINE *ray, COLLISION *result) {
    
    // Get the plane of the face
    PLANE plane;
    face_GetPlane(face, &plane);
    
    // Determine collision with the plane
    if (plane_Collide(&plane, ray, result) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "face_Collide failed: Unable to collide with face plane\n");
#endif
        return FAILURE;
    }
    
    // Determine if the point is in the face
    if (result->how != COLLISION_NONE) {
        if (!face_Contains(face, &result->where)) {
            // It isn't actually in the face even though we collided the plane
            result->how = COLLISION_NONE;
            return SUCCESS;
        }
        
        // Need to set up the proper collision normal now
        if (face_GetNormalAt(face, &result->where, &result->normal) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "face_Collide failed: Unable to interpolate face normal\n");
#endif
            return FAILURE;
        }
    }
    return SUCCESS;
}

/*============================================================*
 * Generalized geometry
 *============================================================*/
int shape_Collide(const SHAPE *shape, const LINE *ray, COLLISION *result) {
    
    // Get collision material
    if (shape->material == NULL) {
#ifdef VERBOSE
        fprintf(stderr, "shape_Collide failed: No material defined for this shape\n");
#endif
        return FAILURE;
    }
    result->material = shape->material;
    
    // Map to specific collision checking functions
    switch (shape->shape) {
    case SHAPE_SPHERE:
        return sphere_Collide((SPHERE *)shape->data, ray, result);
        
    case SHAPE_ELLIPSOID:
        return ellipsoid_Collide((ELLIPSOID *)shape->data, ray, result);
        
    case SHAPE_PLANE:
        return plane_Collide((PLANE *)shape->data, ray, result);
        
    case SHAPE_FACE:
        return face_Collide((FACE *)shape->data, ray, result);
    
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
