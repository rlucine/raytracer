/**********************************************************//**
 * @file parser.inc
 * @brief Implementation of scene file parser
 * @author Rena Shinomiya
 **************************************************************/

#ifndef _PARSER_INC_
#define _PARSER_INC_

// Standard library
#include <stdlib.h>     // malloc, free, realloc, size_t ...
#include <stdio.h>      // fprintf, stderr, fopen, fclose ...
#include <string.h>     // strcmp
#include <ctype.h>      // isspace

// This project
#include "vector.h"     // VECTOR
#include "ppm.h"        // ppm_Decode
#include "image.h"      // IMAGE, TEXCOORD
#include "shape.h"      // SHAPE, PLANE ...
#include "scene.h"      // SCENE
#include "list.h"       // LIST, list_Compress ...
#include "mesh.h"       // MESH, FACE, VERTEX ...

// Debugging modules
#include "debug.h"      // eprintf, assert

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
    LIST shapes;       ///< List of all shapes defined
    LIST lights;       ///< List of all lights defined
    LIST materials;    ///< List of all materials defined
    LIST textures;     ///< List of all textures defined
    LIST vertices;     ///< All vertices loaded
    LIST normals;      ///< All normals loaded
    LIST texcoords;    ///< All texture coordinates loaded
} PARSER_DATA;

/**********************************************************//**
 * @brief Initialize the PARSER_DATA struct for a new file.
 * @param data: The data to initialize.
 * @param scene: The scene location to red into.
 * @return true or false
 **************************************************************/
static bool parser_Create(PARSER_DATA *data, SCENE *scene) {
    
    // Shape data arrays
    if (!list_Create(&data->shapes, sizeof(SHAPE *), 8)) {
        eprintf("Cannot allocate shape data\n");
        return false;
    }
    if (!list_Create(&data->lights, sizeof(LIGHT *), 8)) {
        eprintf("Cannot allocate light data\n");
        return false;
    }
    if (!list_Create(&data->materials, sizeof(MATERIAL *), 8)) {
        eprintf("Cannot allocate material data\n");
        return false;
    }
    if (!list_Create(&data->textures, sizeof(TEXTURE *), 8)) {
        eprintf("Cannot allocate texture data\n");
        return false;
    }
    
    // Mesh data arrays
    if (!list_Create(&data->vertices, sizeof(VECTOR), 8)) {
        eprintf("Cannot allocate vertex table\n");
        return false;
    }
    if (!list_Create(&data->normals, sizeof(VECTOR), 8)) {
        eprintf("Cannot allocate normal table\n");
        return false;
    }
    if (!list_Create(&data->texcoords, sizeof(TEXCOORD), 8)) {
        eprintf("Cannot allocate texture coordinate table\n");
        return false;
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
    
    // Set up current properties
    data->flags = FLAG_NONE;
    data->scene = scene;
    return true;
}

/*============================================================*
 * Helper funcitons
 *============================================================*/
static const char *parse_Vector(const char *string, VECTOR *vector) {
    // Parse a vector from the string
    int delta;
    if (sscanf(string, "%f %f %f%n", &vector->x, &vector->y, &vector->z, &delta) == 3) {
        // Successfully parsed every component
        return string + delta;
    }
    return NULL;
}

static const char *scanner_ParseDouble(const char *string, float *output) {
    int delta;
    if (sscanf(string, "%f%n", output, &delta) == 1) {
        // Successfully parsed the float out
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
    int index = list_Length(&data->materials)-1;
    if (index < 0) {
        return NULL;
    }
    void *location = list_At(&data->materials, index);
    return *((MATERIAL **)location);
}

static TEXTURE *last_Texture(PARSER_DATA *data) {
    // Get the last texture from the data
    int index = list_Length(&data->textures)-1;
    if (index < 0) {
        return NULL;
    }
    void *location = list_At(&data->textures, index);
    return *((TEXTURE **)location);
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
        eprintf("View vector cannot be zero\n");
        return NULL;
    }
    if (data->flags & FLAG_UP && vector_IsColinear(&data->scene->up, &data->scene->view)) {
        eprintf("Up vector parallel to view vector\n");
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
        eprintf("Up vector cannot be zero\n");
        return NULL;
    }
    if (data->flags & FLAG_VIEW && vector_IsColinear(&data->scene->up, &data->scene->view)) {
        eprintf("Up vector parallel to view vector\n");
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
    
    float fov = data->scene->fov;
    if (fov > MAX_FOV || fov < MIN_FOV) {
        eprintf("Fov cannot be %f\n", fov);
        return NULL;
    }
    return result;
}

/*============================================================*
 * Parsing image size
 *============================================================*/
static const char *parse_Size(PARSER_DATA *data, const char *string) {
    float temp;
    const char *where = string;
    
    if ((where = scanner_ParseDouble(where, &temp)) != NULL) {
        data->scene->width = (int)temp;
    } else {
        return NULL;
    }
    if (data->scene->width < 0) {
        eprintf("Invalid width %d\n", data->scene->width);
        return NULL;
    }
    
    if ((where = scanner_ParseDouble(where, &temp)) != NULL) {
        data->scene->height = (int)temp;
        data->flags |= FLAG_SIZE;
        return where;
    }
    if (data->scene->height < 0) {
        eprintf("Invalid height %d\n", data->scene->height);
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
    float exponent;
    
    // Allocate new material
    MATERIAL *material = (MATERIAL *)malloc(sizeof(MATERIAL));
    if (!material) {
        eprintf("Out of memory\n");
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
    failure = failure || (where = scanner_ParseDouble(where, &material->opacity)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &material->refraction)) == NULL;
    if (failure) {
        free(material);
        return NULL;
    }
    
    // Set up material texture (float to int)
    material->exponent = (int)exponent;
    material->texture = last_Texture(data);
    
    // Add new material to array
    assert(sizeof(MATERIAL *) == data->materials.itemSize);
    if (!list_Append(&data->materials, &material)) {
        eprintf("Unable to store new material\n");
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
        eprintf("Invalid radius %f\n", sphere.radius);
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
        eprintf("Missing material definition\n");
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
        eprintf("Out of memory\n");
        return NULL;
    }
    if (!shape_CreateSphere(shape, &sphere, material)) {
        eprintf("Unable to allocate new sphere\n");
        free(shape);
        return NULL;
    }
    
    // Add shape to list
    assert(sizeof(SHAPE *) == data->shapes.itemSize);
    if (!list_Append(&data->shapes, &shape)) {
        eprintf("Unable to store new sphere\n");
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
        eprintf("Invalid dimensions (%f, %f, %f)\n", ellipsoid.dimension.x, ellipsoid.dimension.y, ellipsoid.dimension.z);
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
        eprintf("Missing material definition\n");
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
        eprintf("Out of memory\n");
        return NULL;
    }
    if (!shape_CreateEllipsoid(shape, &ellipsoid, material)) {
        eprintf("Unable to allocate new ellipsoid\n");
        free(shape);
        return NULL;
    }
    
    // Add shape to list
    assert(sizeof(SHAPE *) == data->shapes.itemSize);
    if (!list_Append(&data->shapes, &shape)) {
        eprintf("Unable to store new ellipsoid\n");
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
    float directed;
    
    // Set up light
    LIGHT *light = (LIGHT *)malloc(sizeof(LIGHT));
    if (!light) {
        eprintf("Out of memory\n");
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
            eprintf("Direction cannot be zero\n");
            free(light);
            return NULL;
        }
        
    } else if (directed == 1.0) {
        light->type = LIGHT_POINT;
        memcpy(&light->where, &temp, sizeof(VECTOR));
        
    } else {
        eprintf("Invalid homogeneous coordinate %f\n", directed);
        free(light);
        return NULL;
    }
    
    // Add light to list
    assert(sizeof(LIGHT *) == data->lights.itemSize);
    if (!list_Append(&data->lights, &light)) {
        eprintf("Unable to store new light\n");
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
        eprintf("Out of memory\n");
        return NULL;
    }
    
    // Parse fields
    light->type = LIGHT_SPOT;
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
        eprintf("Direction cannot be zero\n");
        free(light);
        return NULL;
    }
    if (light->angle > MAX_ANGLE || light->angle < MIN_ANGLE) {
        eprintf("Invalid angle %f\n", light->angle);
        free(light);
        return NULL;
    }
    
    // Add light to list
    assert(sizeof(LIGHT *) == data->lights.itemSize);
    if (!list_Append(&data->lights, &light)) {
        eprintf("Unable to store new light\n");
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
        eprintf("Out of memory\n");
        return NULL;
    }
    
    // Try loading the file as a texture
    if (!ppm_Decode(texture, filename)) {
        eprintf("Failed to decode \"%s\"\n", filename);
        free(texture);
        return NULL;
    }

    // Install texture
    if (!list_Append(&data->textures, &texture)) {
        eprintf("Unable to store new texture\n");
        image_Destroy(texture);
        free(texture);
        return NULL;
    }

    // Generating a new texture means we have to push a new material
    // But only if materials defined already!
    if (list_Length(&data->materials) > 0) {
        // Copying the old material
        const MATERIAL *old = last_Material(data);
        MATERIAL *new = (MATERIAL *)malloc(sizeof(MATERIAL));
        if (!new) {
            return NULL;
        }
        memcpy(new, old, sizeof(MATERIAL));
        new->texture = texture;
        
        // Storing new material copy
        if (!list_Append(&data->materials, &new)) {
            eprintf("Unable to store new material\n");
            // Don't deallocate the texture since it's properly in the array now!
            return NULL;
        }
    }
    return where;
}

/*============================================================*
 * Parsing vertex
 *============================================================*/
static const char *parse_Vertex(PARSER_DATA *data, const char *string) {
    const char *where = string;
    
    // Parse data
    VECTOR vertex;
    if ((where = parse_Vector(where, &vertex)) == NULL) {
        return NULL;
    }
    
    // Add vertex to our vertex list
    if (!list_Append(&data->vertices, &vertex)) {
        eprintf("Unable to store new vertex\n");
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
    if (!list_Append(&data->normals, &normal)) {
        eprintf("Unable to store new normal\n");
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
    bool failure = false;
    failure = failure || (where = scanner_ParseDouble(where, &tex.x)) == NULL;
    failure = failure || (where = scanner_ParseDouble(where, &tex.y)) == NULL;
    if (failure) {
        return NULL;
    }
    
    // Add texture coordinate to list
    if (!list_Append(&data->texcoords, &tex)) {
        eprintf("Unable to store new texture coordinate\n");
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
        if (sscanf(where, "%d/%d/%d%n", &vertex->vertex, &vertex->texture, &vertex->normal, &delta) == 3) {
            continue;
        }
        if (sscanf(where, "%d//%d%n", &vertex->vertex, &vertex->normal, &delta) == 2) {
            continue;
        }
        if (sscanf(where, "%d/%d%n", &vertex->vertex, &vertex->texture, &delta) == 2) {
            continue;
        }
        if (sscanf(where, "%d%n", &vertex->vertex, &delta) == 1) {
            continue;
        }
        eprintf("Invalid vertex format\n");
        return NULL;
    }
    
    // Set up material
    MATERIAL *material = last_Material(data);
    if (!material) {
        eprintf("Missing material definition\n");
        return NULL;
    }
    
    // Set up shape
    SHAPE *shape = (SHAPE *)malloc(sizeof(SHAPE));
    if (!shape) {
        eprintf("Out of memory\n");
        return NULL;
    }
    if (!shape_CreateFace(shape, &face, material)) {
        eprintf("Unable to allocate new face\n");
        free(shape);
        return NULL;
    }
    
    // Install face
    if (!list_Append(&data->shapes, &shape)) {
        eprintf("Unable to store new plane\n");
        shape_Destroy(shape);
        free(shape);
        return NULL;
    }
    return where;
}

/**********************************************************//**
 * @def N_KEYWORDS
 * @brief The number of distinct keywords.
 **************************************************************/
#define N_KEYWORDS 16

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
    "v",
    "vn",
    "vt",
    "f",
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
    &parse_Vertex,
    &parse_Normal,
    &parse_TexCoord,
    &parse_Face,
};

/**********************************************************//**
 * @brief Parse one line of data
 * @param data: The data collected by the parser already
 * @param line: The line of characters to parse
 * @return true or false
 **************************************************************/
static bool parser_ParseLine(PARSER_DATA *data, const char *line) {

    // Get the keyword of the line
    size_t size = 32;
    char keyword[size];
    const char *args = scanner_ParseString(line, size, keyword);
    
    // Check if the line is empty
    if (args == NULL) {
        return true;
    }
    
    // The line is a comment
    if (keyword[0] == '#') {
        return true;
    }

    // Check which keyword the line has
    const char *result;
    PARSER parser;
    int i;
    for (i = 0; i < N_KEYWORDS; i++) {
        int length = strlen(KEYWORDS[i]);
        if ((int)strlen(keyword) == length && strncmp(keyword, KEYWORDS[i], length) == 0) {
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
                eprintf("Error parsing %s keyword\n", keyword);
                return false;
            } else if (!scanner_IsEmpty(result)) {
                eprintf("Trailing \"%s\"\n", result);
                return false;
            }
            return true;
        }
    }
    
    // Failed to find a keyword match
    eprintf("Invalid keyword \"%s\"\n", keyword);
    return false;
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
    for (i = 0; i < list_Length(&data->shapes); i++) {
        location = list_At(&data->shapes, i);
        if (location) {
            shape = *(SHAPE **)location;
            shape_Destroy(shape);
            free(shape);
        }
    }
    list_Destroy(&data->shapes);
    
    // Destroy all the lights
    for (i = 0; i < list_Length(&data->lights); i++) {
        location = list_At(&data->lights, i);
        if (location) {
            free(*(LIGHT **)location);
        }
    }
    list_Destroy(&data->lights);
    
    // Destory all the materials
    for (i = 0; i < list_Length(&data->materials); i++) {
        location = list_At(&data->materials, i);
        if (location) {
            free(*(MATERIAL **)location);
        }
    }
    list_Destroy(&data->materials);
    
    // Destroy all the textures
    TEXTURE *texture;
    for (i = 0; i < list_Length(&data->textures); i++) {
        location = list_At(&data->textures, i);
        if (location) {
            texture = *(TEXTURE **)location;
            image_Destroy(texture);
            free(texture);
        }
    }
    list_Destroy(&data->textures);
    
    // Destroy the mesh
    list_Destroy(&data->vertices);
    list_Destroy(&data->normals);
    list_Destroy(&data->texcoords);
}

/*============================================================*
 * Main decoder
 *============================================================*/
bool scene_Decode(SCENE *scene, const char *filename) {
    
    // File reading initialize
    FILE *file = fopen(filename, "r");
    if (!file) {
        eprintf("Cannot open file %s\n", filename);
        return false;
    }
    
    // Set up parser data block
    PARSER_DATA data;
    if (!parser_Create(&data, scene)) {
        eprintf("Failed to allocate parser data block\n");
        fclose(file);
        return false;
    }

    // Read lines from the file
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    int line = 0;
    while (line++, fgets(buf, BUF_SIZE, file)) {
        // Parse this line
        if (!parser_ParseLine(&data, buf)) {
            eprintf("Parse error on line %d\n", line);
            parser_Destroy(&data);
            fclose(file);
            return false;
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
        eprintf("Missing %s definition\n", KEYWORDS[index]);
        parser_Destroy(&data);
        return false;
    }

    // Move shapes arraylist into scene
    list_Compress(&data.shapes);
    if ((scene->nshapes = list_Length(&data.shapes)) > 0) {
        scene->shapes = (SHAPE **)list_Data(&data.shapes);
    } else {
        list_Destroy(&data.shapes);
        scene->shapes = NULL;
    }
    
    // Move lights into scene
    list_Compress(&data.lights);
    if ((scene->nlights = list_Length(&data.lights)) > 0) {
        scene->lights = (LIGHT **)list_Data(&data.lights);
    } else {
        list_Destroy(&data.lights);
        scene->lights = NULL;
    }
    
    // Move materials into scene
    list_Compress(&data.materials);
    if ((scene->nmaterials = list_Length(&data.materials)) > 0) {
        scene->materials = (MATERIAL **)list_Data(&data.materials);
    } else {
        list_Destroy(&data.materials);
        scene->materials = NULL;
    }
    
    // Move textures into scene
    list_Compress(&data.textures);
    if ((scene->ntextures = list_Length(&data.textures)) > 0) {
        scene->textures = (TEXTURE **)list_Data(&data.textures);
    } else {
        list_Destroy(&data.textures);
        scene->textures = NULL;
    }
    
    // Place all mesh data into the mesh
    if ((scene->mesh.nvertices = list_Length(&data.vertices)) > 0) {
        scene->mesh.vertices = (VECTOR *)list_Data(&data.vertices);
    } else {
        list_Destroy(&data.vertices);
        scene->mesh.vertices = NULL;
    }
    if ((scene->mesh.nnormals = list_Length(&data.normals)) > 0) {
        scene->mesh.normals = (VECTOR *)list_Data(&data.normals);
    } else {
        list_Destroy(&data.normals);
        scene->mesh.normals = NULL;
    }
    if ((scene->mesh.ntextures = list_Length(&data.texcoords)) > 0) {
        scene->mesh.texture = (TEXCOORD *)list_Data(&data.texcoords);
    } else {
        list_Destroy(&data.texcoords);
        scene->mesh.texture = NULL;
    }

    // Successfully validated the whole scene
    return true;
}

/*============================================================*/
#endif // _PARSER_INC_
