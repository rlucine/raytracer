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
#include "image.h"
#include "vector.h"
#include "shape.h"
#include "scene.h"
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
    FLAG_SHAPE      = 0x00000180,   // All shapes
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
        line->argc = 3;
    
    } else if (scene_HasKeyword(buf, "sphere", &args)) {
        line->keyword = FLAG_SPHERE;
        line->argc = 4;
    
    } else if (scene_HasKeyword(buf, "ellipsoid", &args)) {
        line->keyword = FLAG_ELLIPSOID;
        line->argc = 6;
    
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
    if (line->argc >= MAX_ARGS) {
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
 * Arraylist of shapes
 *============================================================*/
typedef struct {
    int size;
    int capacity;
    SHAPE *shapes;
} SHAPE_LIST;

/*============================================================*
 * Add a shape to the shape list
 *============================================================*/
static int shapelist_AddShape(SHAPE_LIST *shapes, SHAPE_TYPE type, void *data, MATERIAL *material) {
    
    // Add a new shape to the scene
    if (!shapes->shapes || shapes->size >= shapes->capacity) {
        
        // Check if we can grow (no overflow)
        if (shapes->capacity * 2 < 0) {
#ifdef VERBOSE
            fprintf(stderr, "shapelist_AddShape failed: Too many shapes\n");
#endif
            return FAILURE;
        }
        
        // Grow the shape list
        SHAPE *new;
        if (shapes->capacity > 0) {
            shapes->capacity *= 2;
            new = (SHAPE *)realloc(shapes->shapes, shapes->capacity * sizeof(SHAPE));
        } else {
            shapes->capacity = 1;
            new = (SHAPE *)malloc(shapes->capacity * sizeof(SHAPE));
        }
        
        // Check for memory errors
        if (!new) {
#ifdef VERBOSE
            fprintf(stderr, "shapelist_AddShape failed: Out of memory\n");
#endif
            return FAILURE;
        }
        shapes->shapes = new;
    }
    
    // No problems - deep copy the shape into the scene
    shape_Create(&shapes->shapes[shapes->size++], type, data, material);
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
    
    default:
        return NULL;
    }
    return NULL;
}

/*============================================================*
 * Convert double 0.0-1.0 to RGB 0-255 value
 *============================================================*/
static unsigned char rgb_FromDouble(double d) {
    if (d > 1.0) {
        return RGB_MAX;
    } else if (d < 0.0) {
        return 0;
    }
    return d * RGB_MAX;
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
    if (vector_IsParalell(&scene->up, &scene->view)) {
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
    fprintf(stderr, "scene_Validate: Background color is (%d, %d, %d)\n", scene->background.r, scene->background.g, scene->background.b);
    fprintf(stderr, "scene_Validate: Number of shapes is %d\n", scene->nshapes);
    int n = 0;
    SHAPE_TYPE type;
    const SPHERE *sphere;
    const ELLIPSOID *ellipsoid;
    while (n < scene->nshapes) {
        type = shape_GetGeometry(&scene->shapes[n]);
        switch (type) {
        case SHAPE_SPHERE:
            fprintf(stderr, "scene_Validate: Shape %d is a sphere\n", n);
            sphere = shape_GetSphere(&scene->shapes[n]);
            fprintf(stderr, "scene_Validate: Radius is %lf\n", sphere->radius);
            fprintf(stderr, "scene_Validate: Center is (%lf, %lf, %lf)\n", sphere->center.x, sphere->center.y, sphere->center.z);
            break;
        
        case SHAPE_ELLIPSOID:
            fprintf(stderr, "scene_Validate: Shape %d is an ellipsoid\n", n);
            ellipsoid = shape_GetEllipsoid(&scene->shapes[n]);
            fprintf(stderr, "scene_Validate: Dimensions are (%lf, %lf, %lf)\n", ellipsoid->dimension.x, ellipsoid->dimension.y, ellipsoid->dimension.z);
            fprintf(stderr, "scene_Validate: Center is (%lf, %lf, %lf)\n", ellipsoid->center.x, ellipsoid->center.y, ellipsoid->center.z);
            break;
        
        default:
            fprintf(stderr, "scene_Validate: Shape %d is undefined\n", n);
            break;
        }
        n++;
    }
#endif

    // Successfully validated the whole scene
    return SUCCESS;
}

/*============================================================*
 * Union to store shapes on the stack
 *============================================================*/
typedef union {
    SPHERE sphere;
    ELLIPSOID ellipsoid;
} SHAPE_DATA;

/*============================================================*
 * Main decoder
 *============================================================*/
int scene_Decode(SCENE *scene, const char *filename) {
    
    // List of all shapes in the scene
    SHAPE_LIST shapes;
    shapes.size = 0;
    shapes.capacity = 0;
    shapes.shapes = NULL;
    
    // Shapes to unpack
    SHAPE_DATA data;
    MATERIAL material;
    
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
            scene->background.r = rgb_FromDouble(line.argv[0]);
            scene->background.g = rgb_FromDouble(line.argv[1]);
            scene->background.b = rgb_FromDouble(line.argv[2]);
            break;
        
        case FLAG_MATERIAL:
            // Found material color
            material.color.r = rgb_FromDouble(line.argv[0]);
            material.color.g = rgb_FromDouble(line.argv[1]);
            material.color.b = rgb_FromDouble(line.argv[2]);
            break;
        
        case FLAG_SPHERE:
            // Found a sphere
            data.sphere.center.x = line.argv[0];
            data.sphere.center.y = line.argv[1];
            data.sphere.center.z = line.argv[2];
            data.sphere.radius = line.argv[3];
            
            // Add shape to list
            if (shapelist_AddShape(&shapes, SHAPE_SPHERE, &data.sphere, &material) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Too many shapes\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        case FLAG_ELLIPSOID:
            // Found an ellipsoid
            data.ellipsoid.center.x = line.argv[0];
            data.ellipsoid.center.y = line.argv[1];
            data.ellipsoid.center.z = line.argv[2];
            data.ellipsoid.dimension.x = line.argv[3];
            data.ellipsoid.dimension.y = line.argv[4];
            data.ellipsoid.dimension.z = line.argv[5];
            
            // Add shape to list
            if (shapelist_AddShape(&shapes, SHAPE_ELLIPSOID, &data.ellipsoid, &material) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "scene_Decode failed: Too many shapes\n");
#endif
                failure = 1;
                continue;
            }
            break;
            
        default:
            // Invalid line keyword
#ifdef VERBOSE
            fprintf(stderr, "scene_Decode failed: Invalid like keyword %d\n", line.keyword);
#endif
            failure = 1;
            continue;
        }
    }
    
    // Clean up
    fclose(file);
    
    // Move shapes arraylist into scene
    scene->nshapes = shapes.size;
    scene->shapes = shapes.shapes;

     // Check any failure here to standardize cleanup
    if (failure) {
        // Get rid of shapes arraylist
        scene_Destroy(scene);
        return FAILURE;
    }

    // Check missing flags
    if (!failure) {
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
    }
       
    // Must validate scene
    if (!scene_Validate(scene)) {
        scene_Destroy(scene);
        return FAILURE;
    }
    return SUCCESS;
}

/*============================================================*/
