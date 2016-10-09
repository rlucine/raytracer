/**********************************************************//**
 * @file mesh.c
 * @brief Implementation of 3D triangle meshes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h> // NULL
#include <stdio.h>  // fprintf, stderr ...

// This project
#include "macro.h"  // SUCCESS, FAILURE
#include "vector.h" // VECTOR, POINT ...
#include "image.h"  // TEXCOORD
#include "mesh.h"   // FACE, VERTEX

/*============================================================*
 * Data structure business
 *============================================================*/
static const POINT *face_GetVertex(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int vertex = face->vertices[index].vertex;
    if (vertex < 1 || vertex > face->mesh->nvertices) {
        return NULL;
    }
    return &face->mesh->vertices[vertex];
}

static const VECTOR *face_GetNormal(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int normal = face->vertices[index].normal;
    if (normal == NO_NORMAL || normal < 1 || normal > face->mesh->nnormals) {
        return NULL;
    }
    return &face->mesh->normals[normal];
}

static const TEXCOORD *face_GetTexture(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int texture = face->vertices[index].texture;
    if (texture == NO_TEXTURE || texture < 1 || texture > face->mesh->ntextures) {
        return NULL;
    }
    return &face->mesh->normals[texture-1];
}

/*============================================================*
 * Barycentric coordinates
 *============================================================*/
static int face_GetBarycentricCoordinates(const FACE *face, const POINT *where, VECTOR *barycentric) {
    // Get the total face area
    VECTOR u, v, temp;
    const VECTOR *v0, *v1, *v2;
    v0 = face_GetVertex(face, 0);
    v1 = face_GetVertex(face, 1);
    v2 = face_GetVertex(face, 2);
    if (v0 == NULL || v1 == NULL || v2 == NULL) {
#ifdef VERBOSE
        fprintf(stderr, "face_GetBarycentricCoordinates failed: Illegal vertex index\n");
#endif
        return FAILURE;
    }
    
    vector_Subtract(&u, v1, v0);
    vector_Subtract(&v, v2, v0);
    vector_Cross(&temp, &u, &v);
    double total_area = vector_Magnitude(&temp) / 2.0;
    
    // Get each sub-face area
    vector_Subtract(&u, v1, where);
    vector_Subtract(&v, v2, where);
    vector_Cross(&temp, &u, &v);
    double a = vector_Magnitude(&temp) / 2.0;
    
    vector_Subtract(&u, v0, where);
    vector_Subtract(&v, v2, where);
    vector_Cross(&temp, &u, &v);
    double b = vector_Magnitude(&temp) / 2.0;
    
    vector_Subtract(&u, v0, where);
    vector_Subtract(&v, v1, where);
    vector_Cross(&temp, &u, &v);
    double c = vector_Magnitude(&temp) / 2.0;
    
    // Determine if the point actually in the face
    if ((a+b+c) > total_area) {
        return FAILURE;
    }
    
    // The point is actually within the face!
    if (barycentric) {
        barycentric->x = a / total_area;
        barycentric->y = b / total_area;
        barycentric->z = c / total_area;
    }
    return SUCCESS;
}

/*============================================================*
 * Plane conversion
 *============================================================*/
int face_GetPlane(const FACE *face, PLANE *plane) {
    const VECTOR *v0, *v1, *v2;
    v0 = face_GetVertex(face, 0);
    v1 = face_GetVertex(face, 1);
    v2 = face_GetVertex(face, 2);
    if (v0 == NULL || v1 == NULL || v2 == NULL) {
#ifdef VERBOSE
        fprintf(stderr, "face_GetPlane failed: Illegal vertex index\n");
#endif
        return FAILURE;
    }
    
    // Convert face to plane
    vector_Copy(&plane->origin, v0);
    vector_Subtract(&plane->u, v1, v0);
    vector_Subtract(&plane->v, v2, v0);
    return SUCCESS;
}

/*============================================================*
 * Containing points
 *============================================================*/
int face_Contains(const FACE *face, const POINT *where) {
    return face_GetBarycentricCoordinates(face, where, NULL) == SUCCESS;
}

/*============================================================*
 * Interpolation across the face
 *============================================================*/
int face_GetNormalAt(const FACE *face, const POINT *where, VECTOR *normal) {
    VECTOR barycentric;
    if (face_GetBarycentricCoordinates(face, where, &barycentric) != SUCCESS) {
        return FAILURE;
    }
    
    // If no vertex normals, just use face normal
    const VECTOR *n0, *n1, *n2;
    n0 = face_GetNormal(face, 0);
    n1 = face_GetNormal(face, 1);
    n2 = face_GetNormal(face, 2);
    if (n0 == NULL || n1 == NULL || n2 == NULL) {
        PLANE plane;
        if (face_GetPlane(face, &plane) != SUCCESS) {
            return FAILURE;
        }
        vector_Cross(normal, &plane.u, &plane.v);
        vector_Normalize(normal, normal);
        return SUCCESS;
    }
    
    // Have valid coordinates
    normal->x = normal->y = normal->z = 0.0;
    VECTOR temp;
    
    vector_Multiply(&temp, n0, barycentric.x);
    vector_Add(normal, normal, &temp);
    
    vector_Multiply(&temp, n1, barycentric.y);
    vector_Add(normal, normal, &temp);
    
    vector_Multiply(&temp, n2, barycentric.z);
    vector_Add(normal, normal, &temp);
    return SUCCESS;
}

int face_GetTextureAt(const FACE *face, const POINT *where, TEXCOORD *tex) {
    VECTOR barycentric;
    if (face_GetBarycentricCoordinates(face, where, &barycentric) != SUCCESS) {
        return FAILURE;
    }
    
    // Check if the face even has texture
    const VECTOR *t0, *t1, *t2;
    t0 = face_GetTexture(face, 0);
    t1 = face_GetTexture(face, 1);
    t2 = face_GetTexture(face, 2);
    if (t0 == NULL || t1 == NULL || t2 == NULL) {
        return FAILURE;
    }
    
    // Have valid coordinates
    tex->x = tex->y = tex->z = 0.0;
    VECTOR temp;
    
    vector_Multiply(&temp, t0, barycentric.x);
    vector_Add(tex, tex, &temp);
    
    vector_Multiply(&temp, t1, barycentric.y);
    vector_Add(tex, tex, &temp);
    
    vector_Multiply(&temp, t2, barycentric.z);
    vector_Add(tex, tex, &temp);
    return SUCCESS;
}

/*============================================================*/
