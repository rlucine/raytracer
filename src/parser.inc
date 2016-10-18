/**********************************************************//**
 * @file parser.inc
 * @brief Implementation of scene file parser
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _PARSER_INC_
#define _PARSER_INC_

// Standard library
#include <stdlib.h>     // malloc, free, realloc, size_t ...
#include <stdio.h>      // fprintf, stderr, fopen, fclose ...
#include <string.h>     // strcmp
#include <ctype.h>      // isspace
#include <assert.h>     // assert

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "vector.h"     // VECTOR
#include "ppm.h"        // ppm_Decode
#include "image.h"      // IMAGE, TEXCOORD
#include "shape.h"      // SHAPE, shape_CreateSphere ...
#include "scene.h"      // SCENE
#include "arraylist.h"  // ARRAYLIST, arraylist_Compress ...
#include "mesh.h"       // MESH, FACE, VERTEX ...

// Debugging modules
#include "debug.h"

/**********************************************************//**
 * @enum PARSER_FLAG
 * @brief Defines bitset of all flags that are necessary
 * to define a scene correctly. These bits are managed
 * by each parser function.
 **************************************************************/
typedef enum {
    FLAG_NONE       = 0x00000000,   ///< Empty bitset
    FLAG_EYE        = 0x00000001,   ///< Eye keyword
    FLAG_VIEW       = 0x00000002,   ///< View keyword
    FLAG_UP         = 0x00000004,   ///< Up vector keyword
    FLAG_FOV        = 0x00000008,   ///< FOV keyword
    FLAG_SIZE       = 0x00000010,   ///< Image size keyword
    FLAG_BACKGROUND = 0x00000020,   ///< Background color keyword
    FLAG_ALL        = 0x0000003F,   ///< Full bitset
} PARSER_FLAG;

/**********************************************************//**
 * @struct PARSER_DATA
 * @brief Maintains data about the current state of parsing
 * a scene file. This is intermediate and managed by all the
 * parser funcitons.
 **************************************************************/
typedef struct {
    PARSER_FLAG flags;      ///< Ensure all required keywords found
    SCENE *scene;           ///< The SCENE to read data into
    
    // Shape definitions
    ARRAYLIST shapes;       ///< List of all shapes defined
    ARRAYLIST lights;       ///< List of all lights defined
    ARRAYLIST materials;    ///< List of all materials defined
    ARRAYLIST textures;     ///< List of all textures defined
    ARRAYLIST vertices;     ///< All vertices loaded
    ARRAYLIST normals;      ///< All normals loaded
    ARRAYLIST texcoords;    ///< All texture coordinates loaded
} PARSER_DATA;

/**********************************************************//**
 * @brief Initialize the PARSER_DATA struct for a new file.
 * @param data: The data to initialize.
 * @param scene: The scene location to red into.
 * @return SUCCESS or FAILURE
 **************************************************************/
static int parser_Create(PARSER_DATA *data, SCENE *scene) {
    
    // Shape data arrays
    if (arraylist_Create(&data->shapes, sizeof(SHAPE *), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate shape data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->lights, sizeof(LIGHT *), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate light data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->materials, sizeof(MATERIAL *), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate material data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->textures, sizeof(TEXTURE *), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate texture data\n");
#endif
        return FAILURE;
    }
    
    // Mesh data arrays
    if (arraylist_Create(&data->vertices, sizeof(POINT), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate vertex table\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->normals, sizeof(VECTOR), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate normal table\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->texcoords, sizeof(TEXCOORD), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate texture coordinate table\n");
#endif
        return FAILURE;
    }
    
    // Set up scene properties
    scene->nshapes = 0;
    scene->shapes = NULL;
    scene->nlights = 0;
    scene->lights = NULL;
    scene->nmaterials = 0;
    scene->materials = NULL;
    scene->ntextures = 0;
    scene->textures = NULL;
    scene->flags = PROJECT_PERSPECTIVE;
    
    // Set up current properties
    data->flags = FLAG_NONE;
    data->scene = scene;
    return SUCCESS;
}

/*============================================================*
 * Helper funcitons
 *============================================================*/
static const char *parse_Vector(const char *string, VECTOR *vector) {
    // Parse a vector from the string
    int delta;
    if (sscanf(string, "%lf %lf %lf%n", &vector->x, &vector->y, &vector->z, &delta) == 3) {
        // Successfully parsed every component
        return string + delta;
    }
    return NULL;
}

static const char *scanner_ParseDouble(const char *string, double *output) {
    int delta;
    if (sscanf(string, "%lf%n", output, &delta) == 1) {
        // Successfully parsed the double out
        return string + delta;
    }
    return NULL;
}

static const char *scanner_ParseString(const char *string, size_t size, char *output) {
    char *where = (char *)string;
    size_t i = 0;
    while (*where && isspace(*where)) {
        where++;
    }
    while (*where && i < size-1 && !isspace(*where)) {
        *output++ = *where++;
        i++;
    }
    if (i > 0) {
        // Don't forget this!
        *output = '\0';
        return where;
    }
    return NULL;
}

static int scanner_IsEmpty(const char *string) {
    char *where = (char *)string;
    while (*where) {
        if (!isspace(*where++)) {
            return 0;
        }
    }
    return 1;
}

/*============================================================*
 * Lookup functions
 *============================================================*/
static MATERIAL *last_Material(PARSER_DATA *data) {
    // Get the last material from the data
    void *location = arraylist_End(&data->materials);
    if (!location) {
        return NULL;
    }
    return *((MATERIAL **)location);
}

/*============================================================*
 * Parsing eye
 *============================================================*/
static const char *parse_Eye(PARSER_DATA *data, const char *string) {
    const char *result = parse_Vector(string, &data->scene->eye);
    if (result) {
        data->flags |= FLAG_EYE;
    }
    return result;
}

/*============================================================*
 * Parsing view vector
 *============================================================*/
static const char *parse_View(PARSER_DATA *data, const char *string) {
    const char *result = parse_Vector(string, &data->scene->view);
    if (result) {
        data->flags |= FLAG_VIEW;
    }
    if (vector_IsZero(&data->scene->view)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_View failed: View vector cannot be zero\n");
#endif
        return NULL;
    }
    if (data->flags & FLAG_UP && vector_IsColinear(&data->scene->up, &data->scene->view)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Up failed: Up vector parallel to view vector\n");
#endif
        return NULL;
    }
    
    return result;
}

/*============================================================*
 * Parsing up vector
 *============================================================*/
static const char *parse_Up(PARSER_DATA *data, const char *string) {
    const char *result = parse_Vector(string, &data->scene->up);
    if (result) {
        data->flags |= FLAG_UP;
    }
    if (vector_IsZero(&data->scene->up)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Up failed: Up vector cannot be zero\n");
#endif
        return NULL;
    }
    if (data->flags & FLAG_VIEW && vector_IsColinear(&data->scene->up, &data->scene->view)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Up failed: Up vector parallel to view vector\n");
#endif
        return NULL;
    }
    return result;
}

/*============================================================*
 * Parsing field of view
 *============================================================*/
static const char *parse_Fov(PARSER_DATA *data, const char *string) {
    const char *result = scanner_ParseDouble(string, &data->scene->fov);
    if (result) {
        data->flags |= FLAG_FOV;
    }
    
    double fov = data->scene->fov;
    if (fov > MAX_FOV || fov < MIN_FOV) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Fov failed: Fov cannot be %lf\n", fov);
#endif
        return NULL;
    }
    return result;
}

/*============================================================*
 * Parsing image size
 *============================================================*/
static const char *parse_Size(PARSER_DATA *data, const char *string) {
    double temp;
    const char *where = string;
    
    if ((where = scanner_ParseDouble(where, &temp)) != NULL) {
        data->scene->width = (int)temp;
    } else {
        return NULL;
    }
    if (data->scene->width < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Size failed: Invalid width %d\n", data->scene->width);
#endif
        return NULL;
    }
    
    if ((where = scanner_ParseDouble(where, &temp)) != NULL) {
        data->scene->height = (int)temp;
        data->flags |= FLAG_SIZE;
        return where;
    }
    if (data->scene->height < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Size failed: Invalid height %d\n", data->scene->height);
#endif
        return NULL;
    }
    
    return NULL;
}

/*============================================================*
 * Parsing background color
 *============================================================*/
static const char *parse_Background(PARSER_DATA *data, const char *string) {
    const char *result = parse_Vector(string, &data->scene->background);
    if (result) {
        data->flags |= FLAG_BACKGROUND;
    }
    return result;
}

/*============================================================*
 * Parsing material color
 *============================================================*/
static const char *parse_Material(PARSER_DATA *data, const char *string) {
    const char *where = string;
    double exponent;
    
    // Allocate new material
    MATERIAL *material = (MATERIAL *)malloc(sizeof(MATERIAL));
    if (!material) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Material failed: Out of memory\n");
#endif
        return NULL;
    }
    
    // Parse out material properties
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &material->color)) == NULL;
    failure = failure || (where = parse_Vector(where, &material->highlight)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &material->ambient)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &material->diffuse)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &material->specular)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &exponent)) == NULL;
    if (failure) {
        free(material);
        return NULL;
    }
    
    // Set up material texture (double to int)
    material->exponent = (int)exponent;
    material->texture = NULL;
    
    // Add new material to array
    assert(sizeof(MATERIAL *) == data->materials.itemsize);
    if (arraylist_Append(&data->materials, &material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Material failed: Unable to store new material\n");
#endif
        free(material);
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing sphere
 *============================================================*/
static const char *parse_Sphere(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse data fields
    SPHERE sphere;
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &sphere.center)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &sphere.radius)) == NULL;
    if (failure) {
        return NULL;
    }
    if (sphere.radius < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Invalid radius %lf\n", sphere.radius);
#endif
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Missing material definition\n");
#endif
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Out of memory\n");
#endif
        return NULL;
    }
    if (shape_CreateSphere(shape, &sphere, material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Unable to allocate new sphere\n");
#endif
        free(shape);
        return NULL;
    }
    
    // Add shape to list
    assert(sizeof(SHAPE *) == data->shapes.itemsize);
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Unable to store new sphere\n");
#endif
        shape_Destroy(shape);
        free(shape);
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing ellipsoid
 *============================================================*/
static const char *parse_Ellipsoid(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse out fields
    ELLIPSOID ellipsoid;
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &ellipsoid.center)) == NULL;
    failure = failure || (where = parse_Vector(where, &ellipsoid.dimension)) == NULL;
    if (failure) {
        return NULL;
    }
    if (ellipsoid.dimension.x < 0 || ellipsoid.dimension.y < 0 || ellipsoid.dimension.z < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Invalid dimensions (%lf, %lf, %lf)\n", ellipsoid.dimension.x, ellipsoid.dimension.y, ellipsoid.dimension.z);
#endif
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Missing material definition\n");
#endif
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Out of memory\n");
#endif
        return NULL;
    }
    if (shape_CreateEllipsoid(shape, &ellipsoid, material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to allocate new ellipsoid\n");
#endif
        free(shape);
        return NULL;
    }
    
    // Add shape to list
    assert(sizeof(SHAPE *) == data->shapes.itemsize);
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to store new ellipsoid\n");
#endif
        shape_Destroy(shape);
        free(shape);
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing light
 *============================================================*/
static const char *parse_Light(PARSER_DATA *data, const char *string) {
    const char *where = string;
    VECTOR temp;
    double directed;
    
    // Set up light
    LIGHT *light = (LIGHT *)malloc(sizeof(LIGHT));
    if (!light) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Out of memory\n");
#endif
        return NULL;
    }
    
    // Parse out fields
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &temp)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &directed)) == NULL;
    failure = failure || (where = parse_Vector(where, &light->color)) == NULL;
    if (failure) {
        free(light);
        return NULL;
    }
    if (directed == 0.0) {
        light->type = LIGHT_DIRECTED;
        memcpy(&light->direction, &temp, sizeof(VECTOR));
        if (vector_IsZero(&light->direction)) {
#ifdef VERBOSE
            fprintf(stderr, "parse_Light failed: Direction cannot be zero\n");
#endif
            free(light);
            return NULL;
        }
        
    } else if (directed == 1.0) {
        light->type = LIGHT_POINT;
        memcpy(&light->where, &temp, sizeof(VECTOR));
        
    } else {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Invalid homogeneous coordinate %lf\n", directed);
#endif
        free(light);
        return NULL;
    }
    
    // Add light to list
    assert(sizeof(LIGHT *) == data->lights.itemsize);
    if (arraylist_Append(&data->lights, &light) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Unable to store new light\n");
#endif
        free(light);
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing spotlight
 *============================================================*/
static const char *parse_Spotlight(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Set up light
    LIGHT *light = (LIGHT *)malloc(sizeof(LIGHT));
    if (!light) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Out of memory\n");
#endif
        return NULL;
    }
    
    // Parse fields
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &light->where)) == NULL;
    failure = failure || (where = parse_Vector(where, &light->direction)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &light->angle)) == NULL;
    failure = failure || (where = parse_Vector(where, &light->color)) == NULL;
    if (failure) {
        free(light);
        return NULL;
    }
    if (vector_IsZero(&light->direction)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Spotlight failed: Direction cannot be zero\n");
#endif
        free(light);
        return NULL;
    }
    if (light->angle > MAX_ANGLE || light->angle < MIN_ANGLE) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Invalid angle %lf\n", light->angle);
#endif
        free(light);
        return NULL;
    }
    
    // Add light to list
    assert(sizeof(LIGHT *) == data->lights.itemsize);
    if (arraylist_Append(&data->lights, &light) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Unable to store new light\n");
#endif
        free(light);
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing texture
 *============================================================*/
static const char *parse_Texture(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse fields
    size_t size = 256;
    char filename[size];
    if ((where = scanner_ParseString(where, size, filename)) == NULL) {
        return NULL;
    }
    
    // Load texture
    TEXTURE *texture = (TEXTURE *)malloc(sizeof(TEXTURE));
    if (!texture) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Texture failed: Out of memory\n");
#endif
        return NULL;
    }
    
    // Try loading the file as a texture
    if (ppm_Decode(texture, filename) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Texture failed: Failed to decode \"%s\"\n", filename);
#endif
        free(texture);
        return NULL;
    }

    // Install texture
    if (arraylist_Append(&data->textures, &texture) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Texture failed: Unable to store new texture\n");
#endif
        image_Destroy(texture);
        free(texture);
        return NULL;
    }

    // Generating a new texture means we have to push a new material
    // But only if materials defined already!
    if (arraylist_Length(&data->materials) > 0) {
        // Copying the old material
        const MATERIAL *old = last_Material(data);
        MATERIAL *new = (MATERIAL *)malloc(sizeof(MATERIAL));
        if (!new) {
            return NULL;
        }
        memcpy(new, old, sizeof(MATERIAL));
        new->texture = texture;
        
        // Storing new material copy
        if (arraylist_Append(&data->materials, &new) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "parse_Texture failed: Unable to store new material\n");
#endif
            // Don't deallocate the texture since it's properly in the array now!
            return NULL;
        }
    }
    return where;
}

/*============================================================*
 * Parsing parallel
 *============================================================*/
static const char *parse_Parallel(PARSER_DATA *data, const char *string) {
    // The string should be empty!
    data->scene->flags |= PROJECT_PARALLEL;
    return string;
}

/*============================================================*
 * Parsing plane
 *============================================================*/
#ifdef DEBUG
static const char *parse_Plane(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parsing features
    PLANE plane;
    int failure = 0;
    failure = failure || (where = parse_Vector(where, &plane.origin)) == NULL;
    failure = failure || (where = parse_Vector(where, &plane.u)) == NULL;
    failure = failure || (where = parse_Vector(where, &plane.v)) == NULL;
    if (failure) {
        return NULL;
    }
    if (vector_IsColinear(&plane.u, &plane.v)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Basis vectors are colinear\n");
#endif
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Missing material definition\n");
#endif
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Out of memory\n");
#endif
        return NULL;
    }
    if (shape_CreatePlane(shape, &plane, material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Unable to allocate new plane\n");
#endif
        free(shape);
        return NULL;
    }
    
    // Add shape to list
    if (arraylist_Append(&data->shapes, shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Unable to store new plane\n");
#endif
        shape_Destroy(shape);
        free(shape);
        return NULL;
    }
    return where;
}
#endif

/*============================================================*
 * Parsing vertex
 *============================================================*/
static const char *parse_Vertex(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse data
    POINT vertex;
    if ((where = parse_Vector(where, &vertex)) == NULL) {
        return NULL;
    }
    
    // Add vertex to our vertex list
    if (arraylist_Append(&data->vertices, &vertex) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Vertex failed: Unable to store new vertex\n");
#endif
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing vertex normal
 *============================================================*/
static const char *parse_Normal(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse data
    VECTOR normal;
    if ((where = parse_Vector(where, &normal)) == NULL) {
        return NULL;
    }
    
    // Add normal to our normal list
    if (arraylist_Append(&data->normals, &normal) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Normal failed: Unable to store new normal\n");
#endif
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing texture coordinate
 *============================================================*/
static const char *parse_TexCoord(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse data
    TEXCOORD tex;
    int failure = 0;
    failure = failure || (where = scanner_ParseDouble(where, &tex.x)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &tex.y)) == NULL;
    if (failure) {
        return NULL;
    }
    
    // Add texture coordinate to list
    if (arraylist_Append(&data->texcoords, &tex) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_TexCoord failed: Unable to store new texture coordinate\n");
#endif
        return NULL;
    }
    return where;
}

/*============================================================*
 * Parsing a face
 *============================================================*/
static const char *parse_Face(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Initialize face mesh
    FACE face;
    int i;
    face.mesh = &data->scene->mesh;
    for (i = 0; i < N_VERTICES; i++) {
        face.vertices[i].normal = NO_NORMAL;
        face.vertices[i].texture = NO_TEXTURE;
    }
    
    // Get face vertex values
    int delta;
    VERTEX *vertex;
    for (i = 0; i < N_VERTICES; where += delta, i++) {
        vertex = &face.vertices[i];
        
        // 3 formats for each vertex
        if (sscanf(where, "%d/%d/%d%n", &vertex->vertex, &vertex->normal, &vertex->texture, &delta) == 3) {
            continue;
        }
        if (sscanf(where, "%d//%d%n", &vertex->vertex, &vertex->texture, &delta) == 2) {
            continue;
        }
        if (sscanf(where, "%d/%d%n", &vertex->vertex, &vertex->normal, &delta) == 2) {
            continue;
        }
        if (sscanf(where, "%d%n", &vertex->vertex, &delta) == 1) {
            continue;
        }
#ifdef VERBOSE
        fprintf(stderr, "parse_Face failed: Invalid vertex format\n");
#endif
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Face failed: Missing material definition\n");
#endif
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Face failed: Out of memory\n");
#endif
        return NULL;
    }
    if (shape_CreateFace(shape, &face, material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Face failed: Unable to allocate new face\n");
#endif
        free(shape);
        return NULL;
    }
    
    // Install face
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Face failed: Unable to store new plane\n");
#endif
        shape_Destroy(shape);
        free(shape);
        return NULL;
    }
    return where;
}

/**********************************************************//**
 * @def N_KEYWORDS
 * @brief The number of distinct keywords. There exist keywords
 * that are usable only in DEBUG mode.
 **************************************************************/
#ifdef DEBUG
#define N_KEYWORDS 18
#else
#define N_KEYWORDS 17
#endif

/**********************************************************//**
 * @typedef PARSER
 * @brief Type of all keyword parser functions
 **************************************************************/
// WARNING This line can't be typedef const char * ...
// It has to be typedef char const * for some reason!
typedef char const *(*PARSER)(PARSER_DATA *, const char *);

/**********************************************************//**
 * @var KEYWORDS
 * @brief Array of all keywords as strings
 **************************************************************/
static const char *KEYWORDS[N_KEYWORDS] = {
    "eye",
    "viewdir",
    "updir",
    "fovv",
    "imsize",
    "bkgcolor",
    "mtlcolor",
    "sphere",
    "ellipsoid",
    "light",
    "spotlight",
    "texture",
    "parallel",
    "v",
    "vn",
    "vt",
    "f",
    
#ifdef DEBUG
    // Unnecessary keywords
    "plane",
#endif
};

/**********************************************************//**
 * @var PARSERS
 * @brief Array of all parser functions indexed in the same
 * order as the KEYWORDS array.
 **************************************************************/
static const PARSER PARSERS[N_KEYWORDS] = {
    &parse_Eye,
    &parse_View,
    &parse_Up,
    &parse_Fov,
    &parse_Size,
    &parse_Background,
    &parse_Material,
    &parse_Sphere,
    &parse_Ellipsoid,
    &parse_Light,
    &parse_Spotlight,
    &parse_Texture,
    &parse_Parallel,
    &parse_Vertex,
    &parse_Normal,
    &parse_TexCoord,
    &parse_Face,
    
#ifdef DEBUG
    // Unnecessary keywords
    &parse_Plane,
#endif
};

/**********************************************************//**
 * @brief Parse one line of data
 * @param data: The data collected by the parser already
 * @param line: The line of characters to parse
 * @return SUCCESS or FAILURE
 **************************************************************/
static int parser_ParseLine(PARSER_DATA *data, const char *line) {

    // Get the keyword of the line
    size_t size = 32;
    char keyword[size];
    const char *args = scanner_ParseString(line, size, keyword);
    
    // Check if the line is empty
    if (args == NULL) {
        return SUCCESS;
    }
    
    // The line is a comment
    if (keyword[0] == '#') {
        return SUCCESS;
    }

    // Check which keyword the line has
    const char *result;
    PARSER parser;
    int i;
    for (i = 0; i < N_KEYWORDS; i++) {
        int length = strlen(KEYWORDS[i]);
        if (strlen(keyword) == length && strncmp(keyword, KEYWORDS[i], length) == 0) {
            // Found an equal keyword!
            
            // Delegate to the parser function.
            parser = PARSERS[i];
            if (parser != NULL) {
                result = parser(data, args);
            } else {
                result = NULL;
            }
            
            // Determine result of parser function
            if (result == NULL) {
#ifdef VERBOSE
                fprintf(stderr, "parser_ParseLine failed: Error parsing %s keyword\n", keyword);
#endif
                return FAILURE;
            } else if (!scanner_IsEmpty(result)) {
#ifdef VERBOSE
                fprintf(stderr, "parser_ParseLine failed: Trailing \"%s\"\n", result);
#endif
                return FAILURE;
            }
            return SUCCESS;
        }
    }
    
    // Failed to find a keyword match
#ifdef VERBOSE
    fprintf(stderr, "parser_ParseLine failed: Invalid keyword \"%s\"\n", keyword);
#endif
    return FAILURE;
}

/**********************************************************//**
 * @brief Destroy the parser state. This assumes none of the
 * state has been loaded into the scene yet.
 * @param data: The data to get rid of
 **************************************************************/
void parser_Destroy(PARSER_DATA *data) {
    int i;
    void *location;
    
    // Destroy all the shapes
    SHAPE *shape;
    for (i = 0; i < arraylist_Length(&data->shapes); i++) {
        location = arraylist_At(&data->shapes, i);
        if (location) {
            shape = *(SHAPE **)location;
            shape_Destroy(shape);
            free(shape);
        }
    }
    arraylist_Destroy(&data->shapes);
    
    // Destroy all the lights
    for (i = 0; i < arraylist_Length(&data->lights); i++) {
        location = arraylist_At(&data->lights, i);
        if (location) {
            free(*(LIGHT **)location);
        }
    }
    arraylist_Destroy(&data->lights);
    
    // Destory all the materials
    for (i = 0; i < arraylist_Length(&data->materials); i++) {
        location = arraylist_At(&data->materials, i);
        if (location) {
            free(*(MATERIAL **)location);
        }
    }
    arraylist_Destroy(&data->materials);
    
    // Destroy all the textures
    TEXTURE *texture;
    for (i = 0; i < arraylist_Length(&data->textures); i++) {
        location = arraylist_At(&data->textures, i);
        if (location) {
            texture = *(TEXTURE **)location;
            image_Destroy(texture);
            free(texture);
        }
    }
    arraylist_Destroy(&data->textures);
    
    // Destroy the mesh
    arraylist_Destroy(&data->vertices);
    arraylist_Destroy(&data->normals);
    arraylist_Destroy(&data->texcoords);
}

/*============================================================*
 * Main decoder
 *============================================================*/
int scene_Decode(SCENE *scene, const char *filename) {
    
    // File reading initialize
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef VERBOSE
        perror("fopen");
        fprintf(stderr, "scene_Decode failed: Cannot open file %s\n", filename);
#endif
        return FAILURE;
    }
    
    // Set up parser data block
    PARSER_DATA data;
    if (parser_Create(&data, scene) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Failed to allocate parser data block\n");
#endif
        fclose(file);
        return FAILURE;
    }

    // Read lines from the file
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    int line = 0;
    while (line++, fgets(buf, BUF_SIZE, file)) {
        // Parse this line
        if (parser_ParseLine(&data, buf) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", line);
#endif
            parser_Destroy(&data);
            fclose(file);
            return FAILURE;
        }
    }
    fclose(file);
    
    // Check that all data is defined
    int missing = ~(data.flags) & FLAG_ALL;
    if (missing) {
        int which = 1;
        int index = 0;
        while (!(missing & which)) {
            which <<= 1;
            index++;
        }
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Missing %s definition\n", KEYWORDS[index]);
#endif
        parser_Destroy(&data);
        return FAILURE;
    }

    // Move shapes arraylist into scene
    arraylist_Compress(&data.shapes);
    if ((scene->nshapes = arraylist_Length(&data.shapes)) > 0) {
        scene->shapes = (SHAPE **)arraylist_GetData(&data.shapes);
    } else {
        arraylist_Destroy(&data.shapes);
        scene->shapes = NULL;
    }
    
    // Move lights into scene
    arraylist_Compress(&data.lights);
    if ((scene->nlights = arraylist_Length(&data.lights)) > 0) {
        scene->lights = (LIGHT **)arraylist_GetData(&data.lights);
    } else {
        arraylist_Destroy(&data.lights);
        scene->lights = NULL;
    }
    
    // Move materials into scene
    arraylist_Compress(&data.materials);
    if ((scene->nmaterials = arraylist_Length(&data.materials)) > 0) {
        scene->materials = (MATERIAL **)arraylist_GetData(&data.materials);
    } else {
        arraylist_Destroy(&data.materials);
        scene->materials = NULL;
    }
    
    // Move textures into scene
    arraylist_Compress(&data.textures);
    if ((scene->ntextures = arraylist_Length(&data.textures)) > 0) {
        scene->textures = (TEXTURE **)arraylist_GetData(&data.textures);
    } else {
        arraylist_Destroy(&data.textures);
        scene->textures = NULL;
    }
    
    // Place all mesh data into the mesh
    if ((scene->mesh.nvertices = arraylist_Length(&data.vertices)) > 0) {
        scene->mesh.vertices = (POINT *)arraylist_GetData(&data.vertices);
    } else {
        arraylist_Destroy(&data.vertices);
        scene->mesh.vertices = NULL;
    }
    if ((scene->mesh.nnormals = arraylist_Length(&data.normals)) > 0) {
        scene->mesh.normals = (VECTOR *)arraylist_GetData(&data.normals);
    } else {
        arraylist_Destroy(&data.normals);
        scene->mesh.normals = NULL;
    }
    if ((scene->mesh.ntextures = arraylist_Length(&data.texcoords)) > 0) {
        scene->mesh.texture = (TEXCOORD *)arraylist_GetData(&data.texcoords);
    } else {
        arraylist_Destroy(&data.texcoords);
        scene->mesh.texture = NULL;
    }

#ifdef DEBUG
    // Echo all information in the scene
    fprintf(stderr, "scene_Decode: Eye is (%lf, %lf, %lf)\n", scene->eye.x, scene->eye.y, scene->eye.z);
    fprintf(stderr, "scene_Decode: View is (%lf, %lf, %lf)\n", scene->view.x, scene->view.y, scene->view.z);
    fprintf(stderr, "scene_Decode: Up is (%lf, %lf, %lf)\n", scene->up.x, scene->up.y, scene->up.z);
    fprintf(stderr, "scene_Decode: FovV is %lf\n", scene->fov);
    fprintf(stderr, "scene_Decode: Image size is (%d, %d)\n", scene->width, scene->height);
    fprintf(stderr, "scene_Decode: Background color is (%lf, %lf, %lf)\n", scene->background.x, scene->background.y, scene->background.z);
    
    if (scene->flags & PROJECT_PARALLEL) {
        fprintf(stderr, "scene_Decode: Parallel projection\n");
    } else {
        fprintf(stderr, "scene_Decode: Perspective projection\n");
    }
    
    // Printing mesh
    int m;
    fprintf(stderr, "scene_Decode: %d vertices\n", scene->mesh.nvertices);
    for (m = 0; m < scene->mesh.nvertices; m++) {
        fprintf(stderr, "scene_Decode: Vertex %d is (%lf, %lf, %lf)\n", m+1, scene->mesh.vertices[m].x, scene->mesh.vertices[m].y, scene->mesh.vertices[m].z);
    }
    fprintf(stderr, "scene_Decode: %d normals\n", scene->mesh.nnormals);
    for (m = 0; m < scene->mesh.nnormals; m++) {
        fprintf(stderr, "scene_Decode: Normal %d is (%lf, %lf, %lf)\n", m+1, scene->mesh.normals[m].x, scene->mesh.normals[m].y, scene->mesh.normals[m].z);
    }
    fprintf(stderr, "scene_Decode: %d texture coordinates\n", scene->mesh.ntextures);
    for (m = 0; m < scene->mesh.ntextures; m++) {
        fprintf(stderr, "scene_Decode: Texture coordinate %d is (%lf, %lf)\n", m+1, scene->mesh.texture[m].x, scene->mesh.texture[m].y);
    }
    
    // Printing shapes
    fprintf(stderr, "scene_Decode: Number of shapes is %d\n", scene->nshapes);
    int n = 0;
    const SHAPE *shape;
    const MATERIAL *material;
    const SPHERE *sphere;
    const ELLIPSOID *ellipsoid;
    const PLANE *plane;
    const FACE *face;
    while (n < scene->nshapes) {
        shape = scene_GetShape(scene, n);
        switch (shape->shape) {
        case SHAPE_SPHERE:
            fprintf(stderr, "scene_Decode: Shape %d is a sphere\n", n);
            sphere = shape_GetSphere(shape);
            fprintf(stderr, "scene_Decode: Radius is %lf\n", sphere->radius);
            fprintf(stderr, "scene_Decode: Center is (%lf, %lf, %lf)\n", sphere->center.x, sphere->center.y, sphere->center.z);
            break;
        
        case SHAPE_ELLIPSOID:
            fprintf(stderr, "scene_Decode: Shape %d is an ellipsoid\n", n);
            ellipsoid = shape_GetEllipsoid(shape);
            fprintf(stderr, "scene_Decode: Dimensions are (%lf, %lf, %lf)\n", ellipsoid->dimension.x, ellipsoid->dimension.y, ellipsoid->dimension.z);
            fprintf(stderr, "scene_Decode: Center is (%lf, %lf, %lf)\n", ellipsoid->center.x, ellipsoid->center.y, ellipsoid->center.z);
            break;
            
        case SHAPE_PLANE:
            fprintf(stderr, "scene_Decode: Shape %d is a plane\n", n);
            plane = shape_GetPlane(shape);
            fprintf(stderr, "scene_Decode: Origin is (%lf, %lf, %lf)\n", plane->origin.x, plane->origin.y, plane->origin.z);
            fprintf(stderr, "scene_Decode: Basis u is (%lf, %lf, %lf)\n", plane->u.x, plane->u.y, plane->u.z);
            fprintf(stderr, "scene_Decode: Basis v is (%lf, %lf, %lf)\n", plane->v.x, plane->v.y, plane->v.z);
            break;
            
        case SHAPE_FACE:
            fprintf(stderr, "scene_Decode: Shape %d is a face\n", n);
            face = shape_GetFace(shape);
            int vertex;
            for (vertex = 0; vertex < N_VERTICES; vertex++) {
                fprintf(stderr, "scene_Decode: Vertex %d is %d/%d/%d\n", vertex, face->vertices[vertex].vertex, face->vertices[vertex].normal, face->vertices[vertex].texture);
            }
            break;
        
        default:
            fprintf(stderr, "scene_Decode: Shape %d is undefined\n", n);
            break;
        }
        
        // Print material properties
        material = shape_GetMaterial(shape);
        fprintf(stderr, "scene_Decode: Color is (%lf, %lf, %lf)\n", material->color.x, material->color.y, material->color.z);
        fprintf(stderr, "scene_Decode: Highlight is (%lf, %lf, %lf)\n", material->highlight.x, material->highlight.y, material->highlight.z);
        fprintf(stderr, "scene_Decode: Ka=%lf, Kd=%lf, Ks=%lf, n=%d\n", material->ambient, material->diffuse, material->specular, material->exponent);
        fprintf(stderr, "scene_Decode: Texture is 0x%p\n", material->texture);
        
        n++;
    }
    
    // Printing lights
    fprintf(stderr, "scene_Decode: Number of lights is %d\n", scene->nlights);
    n = 0;
    const LIGHT *light;
    while (n < scene->nlights) {
        light = scene_GetLight(scene, n);
        switch (light->type) {
        case LIGHT_POINT:
            fprintf(stderr, "scene_Decode: Light %d is a point\n", n);
            fprintf(stderr, "scene_Decode: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "scene_Decode: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        case LIGHT_DIRECTED:
            fprintf(stderr, "scene_Decode: Light %d is a direction\n", n);
            fprintf(stderr, "scene_Decode: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "scene_Decode: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
            
        case LIGHT_SPOT:
            fprintf(stderr, "scene_Decode: Light %d is a spotlight\n", n);
            fprintf(stderr, "scene_Decode: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "scene_Decode: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "scene_Decode: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        default:
            fprintf(stderr, "scene_Decode: Light %d is undefined\n", n);
            break;
        }
        n++;
    }
#endif

    // Successfully validated the whole scene
    return SUCCESS;
}

/*============================================================*/
#endif // _PARSER_INC_
