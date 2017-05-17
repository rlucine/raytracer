/**********************************************************//**
 * @file test_mesh.c
 * @brief Test suite for Mesh functions
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...

// This project
#include "vector.h"     // VECTOR
#include "image.h"      // TEXCOORD
#include "mesh.h"       // MESH, FACE ...
#include "debug.h"      // assert

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    int i;
    
    // Verex positions
    VECTOR v[3];
    vector_Set(&v[0], 0, 0, 0);
    vector_Set(&v[1], 1, 0, 0);
    vector_Set(&v[2], 0, 1, 0);
    
    // Normal vectors
    VECTOR n[3];
    vector_Set(&n[0], -1, -1, 1);
    vector_Set(&n[1], 0, 1, 1);
    vector_Set(&n[2], 1, 0, 1);
    
    // Texture coordinates
    VECTOR t[3];
    vector_Set(&t[0], 0, 0, 0);
    vector_Set(&t[1], 0, 1, 0);
    vector_Set(&t[2], 1, 0, 0);
    
    // Set up a mesh
    MESH mesh;
    assert(mesh_Create(&mesh, 3, 3, 3));
    for (i = 0; i < N_VERTICES; i++) {
        mesh.vertices[i] = v[i];
        mesh.normals[i] = n[i];
        mesh.texture[i] = t[i];
    }
    
    // Set up a face
    FACE face;
    face.mesh = &mesh;
    for (int i = 0; i < N_VERTICES; i++) {
        face.vertices[i].vertex = i+1;
        face.vertices[i].normal = i+1;
        face.vertices[i].texture = i+1;
    }
    
    // Test getters
    for (i = 0; i < N_VERTICES; i++) {
        assert(vector_IsEqual(face_GetVertex(&face, i), &v[i]));
        assert(vector_IsEqual(face_GetNormal(&face, i), &n[i]));
        assert(vector_IsEqual(face_GetTexture(&face, i), &t[i]));
    }
    
    // Test containment
    VECTOR extra;
    vector_Set(&extra, 1, 1, 1);
    assert(face_Contains(&face, &v[0]));
    assert(face_Contains(&face, &v[1]));
    assert(face_Contains(&face, &v[2]));
    assert(!face_Contains(&face, &extra));
    
    // Test normal interpolation
    VECTOR test;
    for (i = 0; i < N_VERTICES; i++) {
        assert(face_GetNormalAt(&face, &v[i], &test));
        assert(vector_IsColinear(&n[i], &test));
        assert(face_GetTextureAt(&face, &v[i], &test));
        assert(vector_IsColinear(&t[i], &test));
    }
    
    // Done
    printf("Passed all tests!\n");
    return 0;
}