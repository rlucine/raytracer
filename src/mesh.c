/**********************************************************//**
 * @file mesh.c
 * @brief Implementation of 3D triangle meshes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <stdlib.h>     // NULL
#include <stdio.h>      // fprintf, stderr ...
#include <float.h>      // DBL_EPSILON

// This project
#include "vector.h"     // VECTOR, VECTOR ...
#include "image.h"      // TEXCOORD
#include "mesh.h"       // FACE, VERTEX
#include "geometry.h"   // PLANE, LINE

// Debugging libraries
#include "debug.h"

/*============================================================*
 * Mesh constructor
 *============================================================*/
bool mesh_Create(MESH *mesh, int nvertices, int nnormals, int ntextures) {
    // Allocate vertexes
    if (nvertices > 0) {
        mesh->vertices = (VECTOR *)malloc(nvertices*sizeof(VECTOR));
        if (!mesh->vertices) {
            eprintf("Out of memory\n");
            return false;
        }
    } else {
        mesh->vertices = NULL;
    }
    mesh->nvertices = nvertices;
    
    // Allocate normals
    if (nnormals > 0) {
        mesh->normals = (VECTOR *)malloc(nnormals*sizeof(VECTOR));
        if (!mesh->normals) {
            eprintf("Out of memory\n");
            return false;
        }
    } else {
        mesh->normals = NULL;
    }
    mesh->nnormals = nnormals;
    
    // Allocate texture
    if (ntextures > 0) {
        mesh->texture = (TEXCOORD *)malloc(ntextures*sizeof(TEXCOORD));
        if (!mesh->texture) {
            eprintf("Out of memory\n");
            return false;
        }
    } else {
        mesh->texture = NULL;
    }
    mesh->ntextures = ntextures;
    return true;
}
    
/*============================================================*
 * Mesh destructor
 *============================================================*/
void mesh_Destroy(MESH *mesh) {
    // Free all the mesh data
    if (mesh->nvertices > 0 && mesh->vertices) {
        free(mesh->vertices);
        mesh->nvertices = 0;
        mesh->vertices = NULL;
    }
    if (mesh->nnormals > 0 && mesh->normals) {
        free(mesh->normals);
        mesh->nnormals = 0;
        mesh->normals = NULL;
    }
    if (mesh->ntextures > 0 && mesh->texture) {
        free(mesh->texture);
        mesh->ntextures = 0;
        mesh->texture = NULL;
    }
}

/*============================================================*
 * Data structure business
 *============================================================*/
const VECTOR *face_GetVertex(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int vertex = face->vertices[index].vertex;
    if (vertex < 1 || vertex > face->mesh->nvertices) {
        return NULL;
    }
    return &face->mesh->vertices[vertex-1];
}

const VECTOR *face_GetNormal(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int normal = face->vertices[index].normal;
    if (normal == NO_NORMAL || normal < 1 || normal > face->mesh->nnormals) {
        return NULL;
    }
    return &face->mesh->normals[normal-1];
}

const TEXCOORD *face_GetTexture(const FACE *face, int index) {
    if (index < 0 || index >= N_VERTICES) {
        return NULL;
    }
    int texture = face->vertices[index].texture;
    if (texture == NO_TEXTURE || texture < 1 || texture > face->mesh->ntextures) {
        return NULL;
    }
    return &face->mesh->texture[texture-1];
}

/*============================================================*
 * Barycentric coordinates
 *============================================================*/
static bool face_GetBarycentricCoordinates(const FACE *face, const VECTOR *where, VECTOR *barycentric) {
    // Get the total face area
    VECTOR u, v, temp;
    const VECTOR *v0, *v1, *v2;
    v0 = face_GetVertex(face, 0);
    v1 = face_GetVertex(face, 1);
    v2 = face_GetVertex(face, 2);
    if (v0 == NULL || v1 == NULL || v2 == NULL) {
        eprintf("Illegal vertex index\n");
        return false;
    }
    
    u = *v1;
    vector_Subtract(&u, v0);
    v = *v2;
    vector_Subtract(&v, v0);
    temp = u;
    vector_Cross(&temp, &v);
    float total_area = vector_Length(&temp) / 2.0;
    
    // Get each sub-face area
    u = *v1;
    vector_Subtract(&u, where);
    v = *v2;
    vector_Subtract(&v, where);
    temp = u;
    vector_Cross(&temp, &v);
    float a = vector_Length(&temp) / 2.0;

    u = *where;
    vector_Subtract(&u, v0);
    v = *v2;
    vector_Subtract(&v, v0);
    temp = u;
    vector_Cross(&temp, &v);
    float b = vector_Length(&temp) / 2.0;
    
    u = *v1;
    vector_Subtract(&u, v0);
    v = *where;
    vector_Subtract(&v, v0);
    temp = u;
    vector_Cross(&temp, &v);
    float c = vector_Length(&temp) / 2.0;
    
    // Determine if the point actually in the face
    // TODO if the image becomes grainy increment the multiple of DBL_EPSILON again!
    // TODO just make a macro to do this?
    if ((a+b+c) > total_area+1e-4) {
        return false;
    }
    
    // The point is actually within the face!
    if (barycentric) {
        barycentric->x = a / total_area;
        barycentric->y = b / total_area;
        barycentric->z = c / total_area;
    }
    return true;
}

/*============================================================*
 * Plane conversion
 *============================================================*/
bool face_GetPlane(const FACE *face, PLANE *plane) {
    const VECTOR *v0, *v1, *v2;
    v0 = face_GetVertex(face, 0);
    v1 = face_GetVertex(face, 1);
    v2 = face_GetVertex(face, 2);
    if (v0 == NULL || v1 == NULL || v2 == NULL) {
        eprintf("Illegal vertex index\n");
        return false;
    }
    
    // Convert face to plane
    plane->origin = *v0;
    plane->u = *v1;
    vector_Subtract(&plane->u, v0);
    plane->v = *v2;
    vector_Subtract(&plane->v, v1);
    return true;
}

/*============================================================*
 * Containing points
 *============================================================*/
bool face_Contains(const FACE *face, const VECTOR *where) {
    return face_GetBarycentricCoordinates(face, where, NULL) == true;
}

/*============================================================*
 * Interpolation across the face
 *============================================================*/
bool face_GetNormalAt(const FACE *face, const VECTOR *where, VECTOR *normal) {
    // If no vertex normals, just use face normal
    const VECTOR *n0, *n1, *n2;
    n0 = face_GetNormal(face, 0);
    n1 = face_GetNormal(face, 1);
    n2 = face_GetNormal(face, 2);
    if (n0 == NULL || n1 == NULL || n2 == NULL) {
        PLANE plane;
        if (face_GetPlane(face, &plane) != true) {
            eprintf("Unable to generate normal vector.\n");
            return false;
        }
        *normal = plane.u;
        vector_Cross(normal, &plane.v);
        vector_Normalize(normal);
        return true;
    }
    
    // Interpolate position
    VECTOR barycentric;
    if (face_GetBarycentricCoordinates(face, where, &barycentric) != true) {
        eprintf("Point out of bounds.\n");
        return false;
    }
    
    // Have valid coordinates
    normal->x = normal->y = normal->z = 0.0;
    VECTOR temp;
    
    temp = *n0;
    vector_Multiply(&temp, barycentric.x);
    vector_Add(normal, &temp);
    
    temp = *n1;
    vector_Multiply(&temp, barycentric.y);
    vector_Add(normal, &temp);
    
    temp = *n2;
    vector_Multiply(&temp, barycentric.z);
    vector_Add(normal, &temp);
    
    // Don't bloody forget this
    vector_Normalize(normal);
    return true;
}

bool face_GetTextureAt(const FACE *face, const VECTOR *where, TEXCOORD *tex) {
    // Check if the face even has texture
    if (face->mesh->ntextures == 0) {
        // No texture coordinate on this face.
        return false;
    }
    
    // Make sure alltexture coordinates defined.
    const TEXCOORD *t0, *t1, *t2;
    t0 = face_GetTexture(face, 0);
    t1 = face_GetTexture(face, 1);
    t2 = face_GetTexture(face, 2);
    if (t0 == NULL || t1 == NULL || t2 == NULL) {
        eprintf("Missing texture coordinate.\n");
        return false;
    }
    
    // Interpolate position
    VECTOR barycentric;
    if (face_GetBarycentricCoordinates(face, where, &barycentric) != true) {
        eprintf("Point out of bounds.\n");
        return false;
    }
    
    // Have valid coordinates
    tex->x = tex->y = tex->z = 0.0;
    VECTOR temp;
    
    temp = *t0;
    vector_Multiply(&temp, barycentric.x);
    vector_Add(tex, &temp);
    
    temp = *t1;
    vector_Multiply(&temp, barycentric.y);
    vector_Add(tex, &temp);
    
    temp = *t2;
    vector_Multiply(&temp, barycentric.z);
    vector_Add(tex, &temp);
    return true;
}

/*============================================================*/
