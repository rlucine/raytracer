/**********************************************************//**
 * @file mesh.h
 * @brief Header file for 3D triangle meshes
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _MESH_H_
#define _MESH_H_

#include "vector.h"     // POINT
#include "image.h"      // TEXCOORD

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
    POINT *vertices;    ///< Actual vertex point data
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
    int vertex;         ///< The index of the vertex POINT
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
 * @brief Destory the given polygon mesh.
 * @param mesh: The mesh to destroy.
 **************************************************************/
extern void mesh_Destroy(MESH *mesh);

/**********************************************************//**
 * @brief Get the plane containing the face.
 * @param face: The face to read.
 * @param plane: The location to output the plane.
 **************************************************************/
extern int face_GetPlane(const FACE *face, PLANE *plane);

/**********************************************************//**
 * @brief Check if the face contains the point.
 * @param face: The face to check.
 * @param where: The point to check.
 * @return 1 if true else 0
 **************************************************************/
extern int face_Contains(const FACE *face, const POINT *where);

/**********************************************************//**
 * @brief Interpolate the normal at the given point.
 * @param face: The face to get the normal of.
 * @param where: The point where the normal emanates from.
 * @param normal: The location to write the normal to.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int face_GetNormalAt(const FACE *face, const POINT *where, VECTOR *normal);

/**********************************************************//**
 * @brief Interpolate the texture at the given point.
 * @param face: The face to get the texture from.
 * @param where: The point to compute texture at.
 * @param texture: The place to write the texture coordinate.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int face_GetTextureAt(const FACE *face, const POINT *where, TEXCOORD *texture);

/*============================================================*/
#endif // _MESH_H_