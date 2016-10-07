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

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "vector.h"     // VECTOR
#include "ppm.h"        // ppm_Decode
#include "image.h"      // IMAGE
#include "shape.h"      // SHAPE, shape_CreateSphere ...
#include "scene.h"      // SCENE
#include "arraylist.h"  // ARRAYLIST, arraylist_Compress ...
#include "scanner.h"    // scanner_ParseDouble ...

// Debugging modules
#ifdef DEBUG
#include "tracemalloc.h"
#endif

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
    ARRAYLIST shapes;       ///< List of all shapes defined
    ARRAYLIST lights;       ///< List of all lights defined
    ARRAYLIST materials;    ///< List of all materials defined
    MATERIAL *material;     ///< The most recently defined material
    ARRAYLIST textures;     ///< List of all textures defined
    TEXTURE *texture;       ///< The most recently defined texture
    SCENE *scene;           ///< The SCENE to read data into
} PARSER_DATA;

/**********************************************************//**
 * @brief Initialize the PARSER_DATA struct for a new file.
 * @param data: The data to initialize.
 * @param scene: The scene location to red into.
 * @return SUCCESS or FAILURE
 **************************************************************/
static int parser_Create(PARSER_DATA *data, SCENE *scene) {
    
    // Set up data arrays
    if (arraylist_Create(&data->shapes, sizeof(SHAPE), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate shape data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->lights, sizeof(LIGHT), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate light data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->materials, sizeof(MATERIAL), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate material data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&data->textures, sizeof(TEXTURE), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_Create failed: Cannot allocate texture data\n");
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
    scene->flags = 0;
    
    // Set up current properties
    data->flags = FLAG_NONE;
    data->material = NULL;
    data->texture = NULL;
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
    if (data->flags & FLAG_VIEW && vector_IsColinear(&data->scene->up, &data->scene->view)) {
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
    MATERIAL material;
    double exponent;
    
    if ((where = parse_Vector(where, &material.color)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &material.highlight)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &material.ambient)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &material.diffuse)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &material.specular)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &exponent)) == NULL) {
        return NULL;
    }
    
    // Set up material texture
    material.exponent = (int)exponent;
    material.texture = data->texture;
    
    // Store material in buffer
    if (arraylist_Append(&data->materials, &material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Material failed: Unable to store new material\n");
#endif
        return NULL;
    }
    
    // Set up current material
    data->material = (MATERIAL *)arraylist_At(&data->materials, arraylist_Length(&data->materials)-1);
    return where;
}

/*============================================================*
 * Parsing sphere
 *============================================================*/
static const char *parse_Sphere(PARSER_DATA *data, const char *string) {
    const char *where = string;
    SHAPE shape;
    SPHERE sphere;
    
    if ((where = parse_Vector(where, &sphere.center)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &sphere.radius)) == NULL) {
        return NULL;
    }
    if (sphere.radius < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Invalid radius %lf\n", sphere.radius);
#endif
        return NULL;
    }
    
    // Set up shape
    if (shape_CreateSphere(&shape, &sphere, data->material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Unable to allocate new sphere\n");
#endif
        return NULL;
    }
    
    // Add shape to list
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Sphere failed: Unable to store new sphere\n");
#endif
        shape_Destroy(&shape);
        return NULL;
    }
    
    return where;
}

/*============================================================*
 * Parsing ellipsoid
 *============================================================*/
static const char *parse_Ellipsoid(PARSER_DATA *data, const char *string) {
    const char *where = string;
    SHAPE shape;
    ELLIPSOID ellipsoid;
    
    if ((where = parse_Vector(where, &ellipsoid.center)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &ellipsoid.dimension)) == NULL) {
        return NULL;
    }
    if (ellipsoid.dimension.x < 0 || ellipsoid.dimension.y < 0 || ellipsoid.dimension.z < 0) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Invalid dimensions (%lf, %lf, %lf)\n", ellipsoid.dimension.x, ellipsoid.dimension.y, ellipsoid.dimension.z);
#endif
        return NULL;
    }
    
    // Set up shape
    if (shape_CreateEllipsoid(&shape, &ellipsoid, data->material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to allocate new ellipsoid\n");
#endif
        return NULL;
    }
    
    // Add shape to list
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to store new ellipsoid\n");
#endif
        shape_Destroy(&shape);
        return NULL;
    }
    
    return where;
}

/*============================================================*
 * Parsing light
 *============================================================*/
static const char *parse_Light(PARSER_DATA *data, const char *string) {
    const char *where = string;
    LIGHT light;
    VECTOR temp;
    double directed;
    
    if ((where = parse_Vector(where, &temp)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &directed)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &light.color)) == NULL) {
        return NULL;
    }
    
    // Validate directed light
    if (directed == 0.0) {
        light.type = LIGHT_DIRECTED;
        memcpy(&light.direction, &temp, sizeof(VECTOR));
        if (vector_IsZero(&light.direction)) {
#ifdef VERBOSE
            fprintf(stderr, "parse_Light failed: Direction cannot be zero\n");
#endif
            return NULL;
        }
        
    } else if (directed == 1.0) {
        light.type = LIGHT_POINT;
        memcpy(&light.where, &temp, sizeof(VECTOR));
        
    } else {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Invalid homogeneous coordinate %lf\n", directed);
#endif
        return NULL;
    }
    
    // Add light to list
    if (arraylist_Append(&data->lights, &light) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Unable to store new light\n");
#endif
        return NULL;
    }
    
    return where;
}

/*============================================================*
 * Parsing spotlight
 *============================================================*/
static const char *parse_Spotlight(PARSER_DATA *data, const char *string) {
    const char *where = string;
    LIGHT light;
    
    if ((where = parse_Vector(where, &light.where)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &light.direction)) == NULL) {
        return NULL;
    }
    if ((where = scanner_ParseDouble(where, &light.angle)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &light.color)) == NULL) {
        return NULL;
    }
    
    if (vector_IsZero(&light.direction)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Spotlight failed: Direction cannot be zero\n");
#endif
        return NULL;
    }
    if (light.angle > MAX_ANGLE || light.angle < MIN_ANGLE) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Invalid angle %lf\n", light.angle);
#endif
        return NULL;
    }
    
    // Add light to list
    if (arraylist_Append(&data->lights, &light) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Light failed: Unable to store new light\n");
#endif
        return NULL;
    }
    
    return where;
}

/*============================================================*
 * Parsing texture
 *============================================================*/
static const char *parse_Texture(PARSER_DATA *data, const char *string) {
    size_t size = 256;
    char filename[size];
    const char *where = NULL;
    
    if ((where = scanner_ParseString(string, size, filename)) == NULL) {
        return NULL;
    }
    
    // Try loading the file as a texture
    TEXTURE texture;
    if (ppm_Decode(&texture, filename) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Texture failed: Failed to decode \"%s\"\n", filename);
#endif
        return NULL;
    }

    // Install texture
    if (arraylist_Append(&data->textures, &texture) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Texture failed: Unable to store new texture\n");
#endif
        image_Destroy(&texture);
        return NULL;
    }
    data->texture = (TEXTURE *)arraylist_At(&data->textures, arraylist_Length(&data->textures)-1);

    // Generating a new texture means we have to push a new material
    if (data->material) {
        if (arraylist_Append(&data->materials, data->material) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "parse_Texture failed: Unable to store new material\n");
#endif
            image_Destroy(&texture);
            return NULL;
        }
        data->material = (MATERIAL *)arraylist_At(&data->materials, arraylist_Length(&data->materials)-1);
        
        // Update the new material with the new texture
        data->material->texture = data->texture;
    }
    return where;
}

/*============================================================*
 * Parsing parallel
 *============================================================*/
static const char *parse_Parallel(PARSER_DATA *data, const char *string) {
    data->scene->flags |= PROJECT_PARALLEL;
    return string;
}

/*============================================================*
 * Parsing ellipsoid
 *============================================================*/
static const char *parse_Plane(PARSER_DATA *data, const char *string) {
    const char *where = string;
    SHAPE shape;
    PLANE plane;
    
    if ((where = parse_Vector(where, &plane.origin)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &plane.u)) == NULL) {
        return NULL;
    }
    if ((where = parse_Vector(where, &plane.v)) == NULL) {
        return NULL;
    }
    if (vector_IsColinear(&plane.u, &plane.v)) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Plane failed: Basis vectors are colinear\n");
#endif
        return NULL;
    }
    
    // Set up shape
    if (shape_CreatePlane(&shape, &plane, data->material) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to allocate new plane\n");
#endif
        return NULL;
    }
    
    // Add shape to list
    if (arraylist_Append(&data->shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "parse_Ellipsoid failed: Unable to store new plane\n");
#endif
        shape_Destroy(&shape);
        return NULL;
    }
    
    return where;
}

/**********************************************************//**
 * @def N_KEYWORDS
 * @brief The number of distinct keywords
 **************************************************************/
#define N_KEYWORDS 18

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
    "f",
    "vn",
    "vt",
    
    // Unnecessary keywords
    "plane",
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
    NULL,
    NULL,
    NULL,
    NULL,
    
    // Unnecessary keywords
    &parse_Plane,
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
        if (strncmp(keyword, KEYWORDS[i], strlen(KEYWORDS[i])) == 0) {
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
    
    // Destroy all the shapes
    for (i = 0; i < arraylist_Length(&data->shapes); i++) {
        shape_Destroy((SHAPE *)arraylist_At(&data->shapes, i));
    }
    arraylist_Destroy(&data->shapes);
    
    // Destroy all the lights
    arraylist_Destroy(&data->lights);
    
    // Destory all the materials
    arraylist_Destroy(&data->materials);
    
    // Destroy all the textures
    for (i = 0; i < arraylist_Length(&data->textures); i++) {
        image_Destroy((IMAGE *)arraylist_At(&data->textures, i));
    }
    arraylist_Destroy(&data->textures);
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
        return FAILURE;
    }

    // Set up reading buffer
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    
    // Read lines from the file
    int line = 0;
    while (line++, fgets(buf, BUF_SIZE, file)) {
        // Parse this line
        if (parser_ParseLine(&data, buf) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", line);
#endif
            parser_Destroy(&data);
            return FAILURE;
        }
    }
    
    // Clean up
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
    data.scene->nshapes = arraylist_Length(&data.shapes);
    data.scene->shapes = (SHAPE *)arraylist_GetData(&data.shapes);
    
    // Move lights into scene
    arraylist_Compress(&data.lights);
    data.scene->nlights = arraylist_Length(&data.lights);
    data.scene->lights = (LIGHT *)arraylist_GetData(&data.lights);
    
    if (data.material) {
        printf("First: %lf\n", data.material->color.x);
    }
    
    // Move materials into scene
    arraylist_Compress(&data.materials);
    data.scene->nmaterials = arraylist_Length(&data.materials);
    data.scene->materials = (MATERIAL *)arraylist_GetData(&data.materials);
    
    if (data.material) {
        printf("First: %lf\n", data.material->color.x);
    }
    
    // Move textures into scene
    arraylist_Compress(&data.textures);
    data.scene->ntextures = arraylist_Length(&data.textures);
    data.scene->textures = (TEXTURE *)arraylist_GetData(&data.textures);

#ifdef DEBUG
    // Echo all information in the scene
    fprintf(stderr, "parser_Validate: Eye is (%lf, %lf, %lf)\n", scene->eye.x, scene->eye.y, scene->eye.z);
    fprintf(stderr, "parser_Validate: View is (%lf, %lf, %lf)\n", scene->view.x, scene->view.y, scene->view.z);
    fprintf(stderr, "parser_Validate: Up is (%lf, %lf, %lf)\n", scene->up.x, scene->up.y, scene->up.z);
    fprintf(stderr, "parser_Validate: FovV is %lf\n", scene->fov);
    fprintf(stderr, "parser_Validate: Image size is (%d, %d)\n", scene->width, scene->height);
    fprintf(stderr, "parser_Validate: Background color is (%lf, %lf, %lf)\n", scene->background.x, scene->background.y, scene->background.z);
    
    // Printing shapes
    fprintf(stderr, "parser_Validate: Number of shapes is %d\n", scene->nshapes);
    int n = 0;
    const SHAPE *shape;
    const MATERIAL *material;
    const SPHERE *sphere;
    const ELLIPSOID *ellipsoid;
    while (n < scene->nshapes) {
        shape = scene_GetShape(scene, n);
        switch (shape->shape) {
        case SHAPE_SPHERE:
            fprintf(stderr, "parser_Validate: Shape %d is a sphere\n", n);
            sphere = shape_GetSphere(shape);
            fprintf(stderr, "parser_Validate: Radius is %lf\n", sphere->radius);
            fprintf(stderr, "parser_Validate: Center is (%lf, %lf, %lf)\n", sphere->center.x, sphere->center.y, sphere->center.z);
            break;
        
        case SHAPE_ELLIPSOID:
            fprintf(stderr, "parser_Validate: Shape %d is an ellipsoid\n", n);
            ellipsoid = shape_GetEllipsoid(shape);
            fprintf(stderr, "parser_Validate: Dimensions are (%lf, %lf, %lf)\n", ellipsoid->dimension.x, ellipsoid->dimension.y, ellipsoid->dimension.z);
            fprintf(stderr, "parser_Validate: Center is (%lf, %lf, %lf)\n", ellipsoid->center.x, ellipsoid->center.y, ellipsoid->center.z);
            break;
        
        default:
            fprintf(stderr, "parser_Validate: Shape %d is undefined\n", n);
            break;
        }
        
        // Print material properties
        material = shape_GetMaterial(shape);
        fprintf(stderr, "parser_Validate: Color is (%lf, %lf, %lf)\n", material->color.x, material->color.y, material->color.z);
        fprintf(stderr, "parser_Validate: Highlight is (%lf, %lf, %lf)\n", material->highlight.x, material->highlight.y, material->highlight.z);
        fprintf(stderr, "parser_Validate: Ka=%lf, Kd=%lf, Ks=%lf, n=%d\n", material->ambient, material->diffuse, material->specular, material->exponent);
        fprintf(stderr, "parser_Validate: Texture is 0x%p\n", material->texture);
        
        n++;
    }
    
    // Printing lights
    fprintf(stderr, "parser_Validate: Number of lights is %d\n", scene->nlights);
    n = 0;
    const LIGHT *light;
    while (n < scene->nlights) {
        light = scene_GetLight(scene, n);
        switch (light->type) {
        case LIGHT_POINT:
            fprintf(stderr, "parser_Validate: Light %d is a point\n", n);
            fprintf(stderr, "parser_Validate: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "parser_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        case LIGHT_DIRECTED:
            fprintf(stderr, "parser_Validate: Light %d is a direction\n", n);
            fprintf(stderr, "parser_Validate: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "parser_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
            
        case LIGHT_SPOT:
            fprintf(stderr, "parser_Validate: Light %d is a spotlight\n", n);
            fprintf(stderr, "parser_Validate: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "parser_Validate: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "parser_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        default:
            fprintf(stderr, "parser_Validate: Light %d is undefined\n", n);
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