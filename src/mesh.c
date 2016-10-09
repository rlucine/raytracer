/**********************************************************//**
 * @file mesh.c
 * @brief Implementation of 3D triangle meshes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h> // NULL

// This project
#include "macro.h"  // SUCCESS, FAILURE
#include "vector.h" // VECTOR, POINT ...
#include "image.h"  // TEXCOORD
#include "mesh.h"   // FACE, VERTEX

/*============================================================*
 * Data structure business
 *============================================================*/
static const POINT *face_GetVertex(const FACE *face, int index) {
    // Get the vertex belonging to the face
    return &face->mesh->vertices[face->vertices[index].vertex];
}

static const VECTOR *face_GetNormal(const FACE *face, int index) {
    // Get the vertex normal
    return &face->mesh->normals[face->vertices[index].normal];
}

static const TEXCOORD *face_GetTexture(const FACE *face, int index) {
    // Get the texture coordinate
    return &face->mesh->texture[face->vertices[index].texture];
}

/*============================================================*
 * Barycentric coordinates
 *============================================================*/
static int face_GetBarycentricCoordinates(const FACE *face, const POINT *where, VECTOR *barycentric) {
    // Get the total face area
    VECTOR u, v, temp;
    vector_Subtract(&u, face_GetVertex(face, 1), face_GetVertex(face, 0));
    vector_Subtract(&v, face_GetVertex(face, 2), face_GetVertex(face, 0));
    vector_Cross(&temp, &u, &v);
    double total_area = vector_Magnitude(&temp) / 2.0;
    
    // Get each sub-face area
    vector_Subtract(&u, face_GetVertex(face, 1), where);
    vector_Subtract(&v, face_GetVertex(face, 2), where);
    vector_Cross(&temp, &u, &v);
    double a = vector_Magnitude(&temp) / 2.0;
    
    vector_Subtract(&u, face_GetVertex(face, 0), where);
    vector_Subtract(&v, face_GetVertex(face, 2), where);
    vector_Cross(&temp, &u, &v);
    double b = vector_Magnitude(&temp) / 2.0;
    
    vector_Subtract(&u, face_GetVertex(face, 0), where);
    vector_Subtract(&v, face_GetVertex(face, 1), where);
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
void face_GetPlane(const FACE *face, PLANE *plane) {
    // Convert face to plane
    vector_Copy(&plane->origin, face_GetVertex(face, 0));
    vector_Subtract(&plane->u, face_GetVertex(face, 1), face_GetVertex(face, 0));
    vector_Subtract(&plane->v, face_GetVertex(face, 2), face_GetVertex(face, 0));
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
    
    // Have valid coordinates
    normal->x = normal->y = normal->z = 0.0;
    VECTOR temp;
    
    vector_Multiply(&temp, face_GetNormal(face, 0), barycentric.x);
    vector_Add(normal, normal, &temp);
    
    vector_Multiply(&temp, face_GetNormal(face, 1), barycentric.y);
    vector_Add(normal, normal, &temp);
    
    vector_Multiply(&temp, face_GetNormal(face, 2), barycentric.z);
    vector_Add(normal, normal, &temp);
    return SUCCESS;
}

int face_GetTextureAt(const FACE *face, const POINT *where, TEXCOORD *tex) {
    VECTOR barycentric;
    if (face_GetBarycentricCoordinates(face, where, &barycentric) != SUCCESS) {
        return FAILURE;
    }
    
    // Have valid coordinates
    tex->x = tex->y = tex->z = 0.0;
    VECTOR temp;
    
    vector_Multiply(&temp, face_GetTexture(face, 0), barycentric.x);
    vector_Add(tex, tex, &temp);
    
    vector_Multiply(&temp, face_GetTexture(face, 1), barycentric.y);
    vector_Add(tex, tex, &temp);
    
    vector_Multiply(&temp, face_GetTexture(face, 2), barycentric.z);
    vector_Add(tex, tex, &temp);
    return SUCCESS;
}

/*============================================================*/
