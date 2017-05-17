/**********************************************************//**
 * @file mesh.h
 * @brief Header file for 3D triangle meshes
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _MESH_H_
#define _MESH_H_

// Standard library
#include <stdbool.h>    // bool

// This project
#include "vector.h"     // VECTOR
#include "image.h"      // TEXCOORD
#include "geometry.h"   // PLANE, LINE

/**********************************************************//**
 * @def N_VERTICES
 * @brief How many vertexes each face has
 **************************************************************/
#define N_VERTICES 3

/**********************************************************//**
 * @def NO_NORMAL
 * @brief The vertex has no normal specified
 **************************************************************/
#define NO_NORMAL 0

/**********************************************************//**
 * @def NO_TEXTURE
 * @brief The vertex has no texture coordinate specified
 **************************************************************/
#define NO_TEXTURE 0

/**********************************************************//**
 * @struct MESH
 * @brief Struct for storing triangle mesh data. One SCENE
 * should have just one mesh which defines every face, but each
 * FACE may reside in its own SHAPE.
 **************************************************************/
typedef struct {
    int nvertices;      ///< Number of vertices
    VECTOR *vertices;    ///< Actual vertex point data
    int nnormals;       ///< Number of normals
    VECTOR *normals;    ///< Actual normal vector data
    int ntextures;      ///< Number of texture coordinates
    TEXCOORD *texture;  ///< Actual texture coordinate data
} MESH;

/**********************************************************//**
 * @struct VERTEX
 * @brief Struct for indexing into a MESH
 **************************************************************/
typedef struct {
    int vertex;         ///< The index of the vertex VECTOR
    int normal;         ///< The index of the normal VECTOR
    int texture;        ///< The index of the texture TEXCOORD
} VERTEX;

/**********************************************************//**
 * @struct FACE
 * @brief Struct for storing face data
 **************************************************************/
typedef struct {
    const MESH *mesh;               ///< Pointer to the MESH information
    VERTEX vertices[N_VERTICES];    ///< VERTEX information about the triangle
} FACE;

/**********************************************************//**
 * @brief Create a container for polygon mesh data.
 * @param mesh: The mesh to initialize.
 * @param nvertices: Number of vertexes to buffer.
 * @param nnormals: Number of normals to buffer.
 * @param ntextures: Number of texture coordinates to buffer.
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool mesh_Create(MESH *mesh, int nvertices, int nnormals, int ntextures);

/**********************************************************//**
 * @brief Destory the given polygon mesh.
 * @param mesh: The mesh to destroy.
 **************************************************************/
extern void mesh_Destroy(MESH *mesh);

/**********************************************************//**
 * @brief Get the given vertex from the face.
 * @param face: The face to read.
 * @param index: A vertex identifier from 0 to N_VERTICES-1
 * @return Pointer to the vertex location or NULL.
 **************************************************************/
extern const VECTOR *face_GetVertex(const FACE *face, int index);

/**********************************************************//**
 * @brief Get the given normal vector from the face.
 * @param face: The face to read.
 * @param index: A vertex identifier from 0 to N_VERTICES-1
 * @return Pointer to the a normal vector or NULL.
 **************************************************************/
extern const VECTOR *face_GetNormal(const FACE *face, int index);

/**********************************************************//**
 * @brief Get the given texture coordinate from the face.
 * @param face: The face to read.
 * @param index: A vertex identifier from 0 to N_VERTICES-1
 * @return Pointer to the texture coordinate or NULL.
 **************************************************************/
extern const TEXCOORD *face_GetTexture(const FACE *face, int index);

/**********************************************************//**
 * @brief Get the plane containing the face.
 * @param face: The face to read.
 * @param plane: The location to output the plane.
 * @return Whether a plane could be found.
 **************************************************************/
extern bool face_GetPlane(const FACE *face, PLANE *plane);

/**********************************************************//**
 * @brief Check if the face contains the point.
 * @param face: The face to check.
 * @param where: The point to check.
 * @return 1 if true else 0
 **************************************************************/
extern bool face_Contains(const FACE *face, const VECTOR *where);

/**********************************************************//**
 * @brief Interpolate the normal at the given point.
 * @param face: The face to get the normal of.
 * @param where: The point where the normal emanates from.
 * @param normal: The location to write the normal to.
 * @return Whether a normal could be found.
 **************************************************************/
extern bool face_GetNormalAt(const FACE *face, const VECTOR *where, VECTOR *normal);

/**********************************************************//**
 * @brief Interpolate the texture at the given point.
 * @param face: The face to get the texture from.
 * @param where: The point to compute texture at.
 * @param texture: The place to write the texture coordinate.
 * @return Whether a texture coordinate could be found.
 **************************************************************/
extern bool face_GetTextureAt(const FACE *face, const VECTOR *where, TEXCOORD *texture);

/*============================================================*/
#endif // _MESH_H_