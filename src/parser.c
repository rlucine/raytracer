/**********************************************************//**
 * @file parser.c
 * @brief Implementation of scene file parser
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, realloc, size_t ...
#include <stdio.h>      // fprintf, stderr, fopen, fclose ...
#include <string.h>     // strcmp
#include <ctype.h>      // isspace

// This project
#include "color.h"
#include "image.h"
#include "vector.h"
#include "shape.h"
#include "scene.h"
#include "arraylist.h"
#include "tracemalloc.h"

/*============================================================*
 * Reading flags
 *============================================================*/
typedef enum {
    FLAG_NONE       = 0x00000000,   // Empty flags
    FLAG_EYE        = 0x00000001,   // eye keyword
    FLAG_VIEW       = 0x00000002,   // viewdir keyword
    FLAG_UP         = 0x00000004,   // updir keyword
    FLAG_FOV        = 0x00000008,   // fovv keyword
    FLAG_SIZE       = 0x00000010,   // imsize keyword
    FLAG_BACKGROUND = 0x00000020,   // bkgcolor keyword
    FLAG_REQUIRED   = 0x0000003F,   // All scene files must have these
    FLAG_MATERIAL   = 0x00000040,   // mtlcolor keyword
    FLAG_SPHERE     = 0x00000080,   // sphere keyword
    FLAG_ELLIPSOID  = 0x00000100,   // ellipsoid keyword
    FLAG_LIGHT      = 0x00010000,   // light keyword
    FLAG_SPOTLIGHT  = 0x00020000,   // spotlight keyword
} SCENE_FLAG;

/*============================================================*
 * Tokenized line
 *============================================================*/
#define MAX_ARGS 10

typedef struct {
    SCENE_FLAG keyword;     // Kind of data
    int argc;               // How many arguments?
    double argv[MAX_ARGS];  // Value of arguments
} TOKENS;

/*============================================================*
 * Check if a string has the given keyword
 *============================================================*/
static int scene_HasKeyword(const char *haystack, const char *needle, char **args) {
    int i = 0;
    while (needle[i] && haystack[i]) {
        if (needle[i] != haystack[i]) {
            return 0;
        }
        i++;
    }
    int ret = needle[i] == '\0' && isspace(haystack[i]);
    if (ret && args) {
        *args = (char *)&haystack[i];
    }
    return ret;
}

/*============================================================*
 * Check if a string is only whitespace
 *============================================================*/
static int string_IsAllWhitespace(const char *str) {
    int i = 0;
    while (str[i]) {
        if (!isspace(str[i])) {
            return 0;
        }
        i++;
    }
    return 1;
}

/*============================================================*
 * Parse one line from buf and put tokens in TOKENS
 *============================================================*/
static int scene_ParseLine(TOKENS *line, const char *buf) {
    char *args;
    
    if (scene_HasKeyword(buf, "eye", &args)) {
        line->keyword = FLAG_EYE;
        line->argc = 3;
        
    } else if (scene_HasKeyword(buf, "viewdir", &args)) {
        line->keyword = FLAG_VIEW;
        line->argc = 3;
    
    } else if (scene_HasKeyword(buf, "updir", &args)) {
        line->keyword = FLAG_UP;
        line->argc = 3;
        
    } else if (scene_HasKeyword(buf, "fovv", &args)) {
        line->keyword = FLAG_FOV;
        line->argc = 1;
    
    } else if (scene_HasKeyword(buf, "imsize", &args)) {
        line->keyword = FLAG_SIZE;
        line->argc = 2;
    
    } else if (scene_HasKeyword(buf, "bkgcolor", &args)) {
        line->keyword = FLAG_BACKGROUND;
        line->argc = 3;
    
    } else if (scene_HasKeyword(buf, "mtlcolor", &args)) {
        line->keyword = FLAG_MATERIAL;
        line->argc = 10;
    
    } else if (scene_HasKeyword(buf, "sphere", &args)) {
        line->keyword = FLAG_SPHERE;
        line->argc = 4;
    
    } else if (scene_HasKeyword(buf, "ellipsoid", &args)) {
        line->keyword = FLAG_ELLIPSOID;
        line->argc = 6;
        
    } else if (scene_HasKeyword(buf, "light", &args)) {
        line->keyword = FLAG_LIGHT;
        line->argc = 7;
        
    } else if (scene_HasKeyword(buf, "spotlight", &args)) {
        line->keyword = FLAG_SPOTLIGHT;
        line->argc = 10;
    
    } else if (buf[0] == '#' || string_IsAllWhitespace(buf)) {
        line->keyword = FLAG_NONE;
        line->argc = 0;
        return SUCCESS;
    
    } else {
        // Let someone else print the error condition
#ifdef VERBOSE
        fprintf(stderr, "parser_ParseLine failed: Invalid line \"%s\"\n", buf);
#endif
        return FAILURE;
    }
    
    // Too many?
    if (line->argc > MAX_ARGS) {
#ifdef VERBOSE
        fprintf(stderr, "parser_ParseLine failed: Tell the programmer to raise MAX_ARGS\n");
#endif
        return FAILURE;
    }
    
    // Get all args
    int index = 0;
    int delta;
    while (index < line->argc) {
        if (sscanf(args, "%lf%n", &line->argv[index], &delta) != 1) {
#ifdef VERBOSE
            fprintf(stderr, "parser_ParseLine failed: Required %d arguments but found %d\n", line->argc, index);
#endif
            return FAILURE;
        }
        
        // Move to next argument
        args += delta;
        index++;
    }
    
    // Extra junk after args?
    if (!string_IsAllWhitespace(args)) {
#ifdef VERBOSE
        fprintf(stderr, "parser_ParseLine failed: Extra arguments \"%s\"\n", args);
#endif
        return FAILURE;
    }
    
    // Successfully parsed a line
    return SUCCESS;
}

/*============================================================*
 * Convert flag to its string name
 *============================================================*/
static const char *flag_GetName(int flag) {
    switch (flag) {
    case FLAG_EYE:
        return "eye";
        
    case FLAG_VIEW:
        return "viewdir";
    
    case FLAG_UP:
        return "updir";
    
    case FLAG_FOV:
        return "fovv";
    
    case FLAG_SIZE:
        return "imsize";
    
    case FLAG_BACKGROUND:
        return "bkgcolor";
    
    case FLAG_MATERIAL:
        return "mtlcolor";
    
    case FLAG_SPHERE:
        return "sphere";
    
    case FLAG_ELLIPSOID:
        return "ellipsoid";
        
    case FLAG_LIGHT:
        return "light";
    
    case FLAG_SPOTLIGHT:
        return "spotlight";
    
    default:
        return NULL;
    }
    return NULL;
}

/*============================================================*
 * Validate the entire scene after it has successfully parsed
 *============================================================*/
static int scene_Validate(const SCENE *scene) {
    // Validate scene vectors
    if (vector_IsZero(&scene->view)) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Validate failed: Null view vector\n");
#endif
        return FAILURE;
    }
    if (vector_IsZero(&scene->up)) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Validate failed: Null up vector\n");
#endif
        return FAILURE;
    }
    if (vector_IsColinear(&scene->up, &scene->view)) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Validate failed: Up vector parallel to view vector\n");
#endif
        return FAILURE;
    }
    
    // Validate image size
    if (scene->width <= 0 || scene->height <= 0) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Validate failed: Invalid image size (%d, %d)\n", scene->width, scene->height);
#endif
        return FAILURE;
    }
    
    // Validate fov
    if (scene->fov <= MIN_FOV || scene->fov >= MAX_FOV) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Validate failed: Impossible fov %lf\n", scene->fov);
#endif
        return FAILURE;
    }

#ifdef DEBUG
    fprintf(stderr, "scene_Validate: Eye is (%lf, %lf, %lf)\n", scene->eye.x, scene->eye.y, scene->eye.z);
    fprintf(stderr, "scene_Validate: View is (%lf, %lf, %lf)\n", scene->view.x, scene->view.y, scene->view.z);
    fprintf(stderr, "scene_Validate: Up is (%lf, %lf, %lf)\n", scene->up.x, scene->up.y, scene->up.z);
    fprintf(stderr, "scene_Validate: FovV is %lf\n", scene->fov);
    fprintf(stderr, "scene_Validate: Image size is (%d, %d)\n", scene->width, scene->height);
    fprintf(stderr, "scene_Validate: Background color is (%lf, %lf, %lf)\n", scene->background.x, scene->background.y, scene->background.z);
    
    // Printing shapes
    fprintf(stderr, "scene_Validate: Number of shapes is %d\n", scene->nshapes);
    int n = 0;
    const SHAPE *shape;
    const MATERIAL *material;
    const SPHERE *sphere;
    const ELLIPSOID *ellipsoid;
    while (n < scene->nshapes) {
        shape = scene_GetShape(scene, n);
        switch (shape->shape) {
        case SHAPE_SPHERE:
            fprintf(stderr, "scene_Validate: Shape %d is a sphere\n", n);
            sphere = shape_GetSphere(shape);
            fprintf(stderr, "scene_Validate: Radius is %lf\n", sphere->radius);
            fprintf(stderr, "scene_Validate: Center is (%lf, %lf, %lf)\n", sphere->center.x, sphere->center.y, sphere->center.z);
            break;
        
        case SHAPE_ELLIPSOID:
            fprintf(stderr, "scene_Validate: Shape %d is an ellipsoid\n", n);
            ellipsoid = shape_GetEllipsoid(shape);
            fprintf(stderr, "scene_Validate: Dimensions are (%lf, %lf, %lf)\n", ellipsoid->dimension.x, ellipsoid->dimension.y, ellipsoid->dimension.z);
            fprintf(stderr, "scene_Validate: Center is (%lf, %lf, %lf)\n", ellipsoid->center.x, ellipsoid->center.y, ellipsoid->center.z);
            break;
        
        default:
            fprintf(stderr, "scene_Validate: Shape %d is undefined\n", n);
            continue;
        }
        
        // Print material properties
        material = shape_GetMaterial(shape);
        fprintf(stderr, "scene_Validate: Color is (%lf, %lf, %lf)\n", material->color.x, material->color.y, material->color.z);
        fprintf(stderr, "scene_Validate: Highlight is (%lf, %lf, %lf)\n", material->highlight.x, material->highlight.y, material->highlight.z);
        fprintf(stderr, "scene_Validate: Ka=%lf, Kd=%lf, Ks=%lf, n=%d\n", material->ambient, material->diffuse, material->specular, material->exponent);
        
        n++;
    }
    
    // Printing lights
    fprintf(stderr, "scene_Validate: Number of lights is %d\n", scene->nlights);
    n = 0;
    const LIGHT *light;
    while (n < scene->nlights) {
        light = scene_GetLight(scene, n);
        switch (light->type) {
        case LIGHT_POINT:
            fprintf(stderr, "scene_Validate: Light %d is a point\n", n);
            fprintf(stderr, "scene_Validate: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "scene_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        case LIGHT_DIRECTED:
            fprintf(stderr, "scene_Validate: Light %d is a direction\n", n);
            fprintf(stderr, "scene_Validate: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "scene_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
            
        case LIGHT_SPOT:
            fprintf(stderr, "scene_Validate: Light %d is a spotlight\n", n);
            fprintf(stderr, "scene_Validate: Position is (%lf, %lf, %lf)\n", light->where.x, light->where.y, light->where.z);
            fprintf(stderr, "scene_Validate: Direction is (%lf, %lf, %lf)\n", light->direction.x, light->direction.y, light->direction.z);
            fprintf(stderr, "scene_Validate: Color is (%lf, %lf, %lf)\n", light->color.x, light->color.y, light->color.z);
            break;
        
        default:
            fprintf(stderr, "scene_Validate: Light %d is undefined\n", n);
            break;
        }
        n++;
    }
#endif

    // Successfully validated the whole scene
    return SUCCESS;
}

/*============================================================*
 * Main decoder
 *============================================================*/
int scene_Decode(SCENE *scene, const char *filename) {
    
    // Set up scene so we don't free garbage on failure
    scene->nshapes = 0;
    scene->shapes = NULL;
    scene->nlights = 0;
    scene->lights = NULL;
    
    // Set up data arrays
    ARRAYLIST shapes, lights;
    if (arraylist_Create(&shapes, sizeof(SHAPE), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Cannot allocate shape data\n");
#endif
        return FAILURE;
    }
    if (arraylist_Create(&lights, sizeof(LIGHT), 8) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Cannot allocate light data\n");
#endif
        return FAILURE;
    }
    
    // Shapes to unpack
    SHAPE shape;
    ELLIPSOID ellipsoid;
    SPHERE sphere;
    MATERIAL material;
    LIGHT light;
    
    // Parser flags
    int flags = 0;
    
    // File reading initialize
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Cannot open %s\n", filename);
#endif
        return FAILURE;
    }

    // Set up reading buffer
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    
    // Read lines from the file
    TOKENS line;
    int lineno = 0;
    int failure = 0;
    while (lineno++, !failure && fgets(buf, BUF_SIZE, file)) {
        // Parse this line
        if (scene_ParseLine(&line, buf) != SUCCESS) {
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", lineno);
#endif
            failure = 1;
            break;
        }
        
        // Register flag
        if (flags & line.keyword & FLAG_REQUIRED) {
            // Redefining a required field
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Multiple definition of %s on line %d\n", flag_GetName(line.keyword), lineno);
#endif
            failure = 1;
            break;
        }
        flags |= line.keyword;
        
        // Install data into scene
        switch (line.keyword) {
        case FLAG_NONE:
            // Skip this line
            continue;
            
        case FLAG_EYE:
            // Found eye definition
            scene->eye.x = line.argv[0];
            scene->eye.y = line.argv[1];
            scene->eye.z = line.argv[2];
            break;
        
        case FLAG_VIEW:
            // Found view vector
            scene->view.x = line.argv[0];
            scene->view.y = line.argv[1];
            scene->view.z = line.argv[2];
            break;
        
        case FLAG_UP:
            // Found up vector
            scene->up.x = line.argv[0];
            scene->up.y = line.argv[1];
            scene->up.z = line.argv[2];
            break;
        
        case FLAG_FOV:
            // Found fovv
            scene->fov = line.argv[0];
            break;
        
        case FLAG_SIZE:
            // Found image size
            scene->width = line.argv[0];
            scene->height = line.argv[1];
            break;
        
        case FLAG_BACKGROUND:
            // Found background color
            scene->background.x = line.argv[0];
            scene->background.y = line.argv[1];
            scene->background.z = line.argv[2];
            break;
        
        case FLAG_MATERIAL:
            // Found material color
            material.color.x = line.argv[0];
            material.color.y = line.argv[1];
            material.color.z = line.argv[2];
            material.highlight.x = line.argv[3];
            material.highlight.y = line.argv[4];
            material.highlight.z = line.argv[5];
            material.ambient = line.argv[6];
            material.diffuse = line.argv[7];
            material.specular = line.argv[8];
            material.exponent = (int)line.argv[9];
            break;
        
        case FLAG_SPHERE:
            // Found a sphere
            sphere.center.x = line.argv[0];
            sphere.center.y = line.argv[1];
            sphere.center.z = line.argv[2];
            sphere.radius = line.argv[3];
            
            // Generate shape to copy in to list
            if (shape_CreateSphere(&shape, &sphere, &material) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to allocate new sphere\n");
#endif
                failure = 1;
                continue;
            }
            
            // Add shape to list
            if (arraylist_Append(&shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to store new sphere\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        case FLAG_ELLIPSOID:
            // Found an ellipsoid
            ellipsoid.center.x = line.argv[0];
            ellipsoid.center.y = line.argv[1];
            ellipsoid.center.z = line.argv[2];
            ellipsoid.dimension.x = line.argv[3];
            ellipsoid.dimension.y = line.argv[4];
            ellipsoid.dimension.z = line.argv[5];
            
            // Generate shape to copy in to list
            if (shape_CreateEllipsoid(&shape, &ellipsoid, &material) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to allocate new ellipsoid\n");
#endif
                failure = 1;
                continue;
            }
            
            // Add shape to list
            if (arraylist_Append(&shapes, &shape) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to store new sphere\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        case FLAG_LIGHT:
            // Found a point or directed light
            if (line.argv[3] == 0.0) {
                // Directed light
                light.type = LIGHT_DIRECTED;
                light.direction.x = line.argv[0];
                light.direction.y = line.argv[1];
                light.direction.z = line.argv[2];
            } else {
                // Point light
                light.type = LIGHT_POINT;
                light.where.x = line.argv[0];
                light.where.y = line.argv[1];
                light.where.z = line.argv[2];
            }
            light.color.x = line.argv[4];
            light.color.y = line.argv[5];
            light.color.z = line.argv[6];
            
            // Add light to list
            if (arraylist_Append(&lights, &light) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to store new light\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        case FLAG_SPOTLIGHT:
            // Found a spotlight
            light.type = LIGHT_SPOT;
            light.where.x = line.argv[0];
            light.where.y = line.argv[1];
            light.where.z = line.argv[2];
            light.direction.x = line.argv[3];
            light.direction.y = line.argv[4];
            light.direction.z = line.argv[5];
            light.angle = line.argv[6];
            light.color.x = line.argv[7];
            light.color.y = line.argv[8];
            light.color.z = line.argv[9];
            
            // Add light to list
            if (arraylist_Append(&lights, &light) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Unable to store new light\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        default:
            // Invalid line keyword
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Invalid line keyword %d\n", line.keyword);
#endif
            failure = 1;
            continue;
        }
    }
    
    // Clean up
    fclose(file);
    
    // Move shapes arraylist into scene
    arraylist_Compress(&shapes);
    scene->nshapes = arraylist_Length(&shapes);
    scene->shapes = (SHAPE *)arraylist_GetData(&shapes);
    
    // Move lights into scene
    arraylist_Compress(&lights);
    scene->nlights = arraylist_Length(&lights);
    scene->lights = (LIGHT *)arraylist_GetData(&lights);

     // Check any failure here to standardize cleanup
    if (failure) {
        // Get rid of shapes arraylist
        scene_Destroy(scene);
        return FAILURE;
    }

    // Check missing flags
    int missing = ~flags & FLAG_REQUIRED;
    if (missing) {
        int which = 1;
        while (!(missing & which)) {
            which <<= 1;
        }
#ifdef VERBOSE
        fprintf(stderr, "scene_Decode failed: Missing %s definition\n", flag_GetName(which));
#endif
        scene_Destroy(scene);
        return FAILURE;
    }
       
    // Must validate scene
    if (scene_Validate(scene) != SUCCESS) {
        scene_Destroy(scene);
        return FAILURE;
    }
    return SUCCESS;
}

/*============================================================*/
