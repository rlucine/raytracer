/**********************************************************//**
 * @file shape.c
 * @brief Implementation of 3D shape geometry
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <stdlib.h>     // free
#include <string.h>     // memcpy
#include <math.h>       // sqrt, asin, M_PI, fabs, sqrt ...
#include <stdio.h>      // fprintf, stderr ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON

// This project
#include "geometry.h"   // PLANE, LINE
#include "image.h"      // IMAGE
#include "vector.h"     // VECTOR
#include "shape.h"      // SHAPE
#include "mesh.h"       // MESH, FACE ...

// Debugging modules
#include "debug.h"

/*============================================================*
 * Shape creation
 *============================================================*/
bool shape_CreateSphere(SHAPE *shape, const SPHERE *sphere, const MATERIAL *material) {
    // Initialize a new sphere
    shape->shape = SHAPE_SPHERE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(SPHERE));
    if (!shape->data) {
        eprintf("Out of memory\n");
        return false;
    }
    memcpy(shape->data, sphere, sizeof(SPHERE));
    
    // Copy material into shape
    shape->material = material;
    return true;
}

bool shape_CreateEllipsoid(SHAPE *shape, const ELLIPSOID *ellipsoid, const MATERIAL *material) {
    // Initialize a new ellipsoid
    shape->shape = SHAPE_ELLIPSOID;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(ELLIPSOID));
    if (!shape->data) {
        eprintf("Out of memory\n");
        return false;
    }
    memcpy(shape->data, ellipsoid, sizeof(ELLIPSOID));
    
    // Copy material into shape
    shape->material = material;
    return true;
}

bool shape_CreatePlane(SHAPE *shape, const PLANE *plane, const MATERIAL *material) {
    // Initialize a new infinite plane
    shape->shape = SHAPE_PLANE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(PLANE));
    if (!shape->data) {
        eprintf("Out of memory\n");
        return false;
    }
    memcpy(shape->data, plane, sizeof(PLANE));
    
    // Copy material into shape
    shape->material = material;
    return true;
}

bool shape_CreateFace(SHAPE *shape, const FACE *face, const MATERIAL *material) {
    // Initialize a new triangle face
    shape->shape = SHAPE_FACE;
    
    // Copy data into the shape
    shape->data = malloc(sizeof(FACE));
    if (!shape->data) {
        eprintf("Out of memory\n");
        return false;
    }
    memcpy(shape->data, face, sizeof(FACE));
    
    // Copy material into shape
    shape->material = material;
    return true;
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
static bool sphere_Collide(const SPHERE *sphere, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
        eprintf("direction is the null vector\n");
        return false;
    }
    if (sphere->radius <= 0.0) {
        eprintf("Sphere radius nonpositive\n");
        return false;
    }

    // Solve for sphere intersection with line
    VECTOR unit_direction = ray->direction;
    vector_Normalize(&unit_direction);
    float a = 1.0;
    VECTOR dis_center = ray->origin;
    vector_Subtract(&dis_center, &sphere->center);
    float b = 2.0*vector_Dot(&unit_direction, &dis_center);
    float c = vector_Dot(&dis_center, &dis_center) - (sphere->radius * sphere->radius);

    // Solve the quadratic att + bt + c = 0
    float discriminant = b*b - 4.0*a*c;
    if (discriminant < 0.0) {
        // No solutions - missed the sphere
        result->how = COLLISION_NONE;
        return true;
    }

    // Two or one solution
    float t1, t2;
    t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    t2 = (-b - sqrt(discriminant)) / (2.0 * a);

    // Determine closest collision
    float tclosest = 0.0;
    if (t1 < t2 && t1 >= 0) {
        tclosest = t1;
    } else if (t2 >= 0) {
        tclosest = t2;
    } else {
        // The sphere is behind the viewer - miss!
        result->how = COLLISION_NONE;
        return true;
    }

    // Get location of closest collision
    if (vector_Length(&dis_center) <= sphere->radius) {
        result->how = COLLISION_INSIDE;
    } else {
        result->how = COLLISION_SURFACE;
    }
    result->distance = tclosest;
    result->where = unit_direction;
    vector_Multiply(&result->where, tclosest);
    vector_Add(&result->where, &ray->origin);
    
    // Get normal vector at collision
    result->normal = result->where;
    vector_Subtract(&result->normal, &sphere->center);
    vector_Normalize(&result->normal);
    
    // Get the texture at the collision site
    result->texcoord.x = atan2(result->normal.x, result->normal.z) / (2*M_PI);
    if (result->texcoord.x < 0.0) {
        result->texcoord.x += 1.0;
    }
    result->texcoord.y = acos(result->normal.y)/M_PI;
    result->texcoord.z = 0.0;
    
    // Done!
    return true;
}

/*============================================================*
 * Ellipsoid geometry
 *============================================================*/
static bool ellipsoid_Collide(const ELLIPSOID *ellipsoid, const LINE *ray, COLLISION *result) {
    
    // Error checking
    if (vector_IsZero(&ray->direction)) {
        eprintf("direction is the null vector\n");
        return false;
    }
    
    const VECTOR *dimension = &ellipsoid->dimension;
    if (dimension->x <= 0.0 || dimension->y <= 0.0 || dimension->z <= 0.0) {
        eprintf("Ellipsoid with negative dimension\n");
        return false;
    }

    // Get the unit direction of the ray
    VECTOR unit = ray->direction;
    vector_Normalize(&unit);
    
    // Get the distance to the center of the ellipsoid
    VECTOR center = ray->origin;
    vector_Subtract(&center, &ellipsoid->center);
    
    float a = 0.0;
    a += unit.x*unit.x / (dimension->x * dimension->x);
    a += unit.y*unit.y / (dimension->y * dimension->y);
    a += unit.z*unit.z / (dimension->z * dimension->z);
    
    float b = 0.0;
    b += 2*(center.x * unit.x / (dimension->x * dimension->x));
    b += 2*(center.y * unit.y / (dimension->y * dimension->y));
    b += 2*(center.z * unit.z / (dimension->z * dimension->z));
    
    float c = -1.0;
    c += center.x*center.x / (dimension->x * dimension->x);
    c += center.y*center.y / (dimension->y * dimension->y);
    c += center.z*center.z / (dimension->z * dimension->z);
    
    // Solve the quadratic att + bt + c = 0
    float discriminant = b*b - 4.0*a*c;
    if (discriminant < 0.0) {
        // No solutions - missed the sphere
        result->how = COLLISION_NONE;
        return true;
    }

    // Two or one solution
    float t1, t2;
    t1 = (-b + sqrt(discriminant)) / (2.0 * a);
    t2 = (-b - sqrt(discriminant)) / (2.0 * a);

    // Determine closest collision
    float tclosest = 0.0;
    if (t1 < t2 && t1 >= 0) {
        tclosest = t1;
    } else if (t2 >= 0) {
        tclosest = t2;
    } else {
        // The sphere is behind the viewer - miss!
        result->how = COLLISION_NONE;
        return true;
    }

    // Get location of closest collision
    // This absorbs case when we collide immediately
    result->how = COLLISION_SURFACE;
    result->distance = tclosest;
    result->where = unit;
    vector_Multiply(&result->where, tclosest);
    vector_Add(&result->where, &ray->origin);
    
    // Get the normal vector at the collision site
    result->normal = result->where;
    vector_Subtract(&result->normal, &ellipsoid->center);
    result->normal.x *= 2.0 / (dimension->x * dimension->x);
    result->normal.y *= 2.0 / (dimension->y * dimension->y);
    result->normal.z *= 2.0 / (dimension->z * dimension->z);
    vector_Normalize(&result->normal);
    
    // TODO texture an ellipsoid!
    vector_Set(&result->texcoord, 0, 0, 0);
    return true;
}

/*============================================================*
 * Plane geometry
 *============================================================*/
static bool plane_Collide(const PLANE *plane, const LINE *ray, COLLISION *result) {
    
    // Collide the ray with the plane
    if (vector_IsZero(&ray->direction)) {
        eprintf("direction is the null vector\n");
        return false;
    }
    
    // Get the unit direction of the ray
    VECTOR unit = ray->direction;
    vector_Normalize(&unit);
    
    // Get the plane's normal vector
    VECTOR normal = plane->u;
    vector_Cross(&normal, &plane->v);
    vector_Normalize(&normal);
    
    // Determine where the ray and plane intersect
    VECTOR offset = plane->origin;
    vector_Subtract(&offset, &ray->origin);
    float tclosest, numerator, denominator = vector_Dot(&normal, &unit);
    if (fabs(denominator) <= DBL_EPSILON) {
        if (fabs(vector_Dot(&offset, &normal)) <= DBL_EPSILON) {
            // Inside plane
            numerator = 0.0;
            tclosest = 0.0;
        } else {
            // Paralell to plane - no collision
            result->how = COLLISION_NONE;
            return true;
        }
    } else {
        numerator = vector_Dot(&normal, &offset);
        tclosest = numerator / denominator;
    }
    
    // Determine collision location
    if (tclosest < 0.0) {
        // The plane is behind the viewer - miss!
        result->how = COLLISION_NONE;
        return true;
    } else if (tclosest == 0.0) {
        // Rarely happens
        result->how = COLLISION_INSIDE;
    } else {
        result->how = COLLISION_SURFACE;
    }
    
    // Determine where the collision is
    result->distance = tclosest;
    result->where = unit;
    vector_Multiply(&result->where, tclosest);
    vector_Add(&result->where, &ray->origin);
    
    // Determine normal at collision site
    result->normal = normal;
    
    // TODO Texture the plane!
    vector_Set(&result->texcoord, 0, 0, 0);
    return true;
}

/*============================================================*
 * Face geometry
 *============================================================*/
static bool face_Collide(const FACE *face, const LINE *ray, COLLISION *result) {
    
    // Get the plane of the face
    PLANE plane;
    if (face_GetPlane(face, &plane) != true) {
        eprintf("Unable to generate face plane\n");
        return false;
    }
    
    // Determine collision with the plane
    if (plane_Collide(&plane, ray, result) != true) {
        eprintf("Unable to collide with face plane\n");
        return false;
    }
    
    // Determine if the point is in the face
    if (result->how != COLLISION_NONE) {
        if (!face_Contains(face, &result->where)) {
            // It isn't actually in the face even though we collided the plane
            result->how = COLLISION_NONE;
            return true;
        }
        
        // Need to set up the proper collision normal now
        if (face_GetNormalAt(face, &result->where, &result->normal) != true) {
            eprintf("Unable to interpolate face normal\n");
            return false;
        }
        
        // Set up texture coordinate if the face is textured at all
        if (result->texture && face_GetTextureAt(face, &result->where, &result->texcoord) != true) {
            eprintf("Texture defined but missing texture coordinates\n");
            return false;
        }
    }
    
    return true;
}

/*============================================================*
 * Generalized geometry
 *============================================================*/
bool shape_Collide(const SHAPE *shape, const LINE *ray, COLLISION *result) {
    
    // Get collision material
    if (shape->material == NULL) {
        eprintf("No material defined for this shape\n");
        return false;
    }
    result->material = shape->material;
    result->texture = shape->material->texture;
    
    // Get incident direction
    result->incident = ray->direction;
    vector_Negate(&result->incident);
    vector_Normalize(&result->incident);

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
    
    eprintf("No collision for shape type %d\n", shape->shape);
    return false;
}

/*============================================================*
 * Texture shader
 *============================================================*/
bool shape_GetColorAt(const COLLISION *collision, COLOR *color) {
    
    // Get the diffuse color
    if (collision->material->texture != NULL) {
        if (image_GetTexture(collision->material->texture, &collision->texcoord, color) != true) {
            eprintf("Unable to access texture data\n");
            return false;
        }
    } else {
        memcpy(color, &collision->material->color, sizeof(COLOR));
    }
    return true;
}

/*============================================================*/
