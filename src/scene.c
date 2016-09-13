/**********************************************************//**
 * @file scene.c
 * @brief Implementation file for raytracer scenes
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, realloc, size_t ...
#include <stdio.h>      // fprintf, stderr, fopen, fclose ...
#include <string.h>     // strcmp
#include <ctype.h>      // isspace
#include <float.h>      // DBL_EPSILON

// This project
#include "rgb.h"
#include "vector.h"
#include "scene.h"

/*============================================================*
 * Decoder state
 *============================================================*/

#define FLAG_EYE 1
#define FLAG_VIEW 2
#define FLAG_UP 4
#define FLAG_FOV 8
#define FLAG_SIZE 16
#define FLAG_BACKGROUND 32
#define FLAG_MATERIEL 64
#define FLAG_ALL 127

/*============================================================*
 * Add shapes to scene
 *============================================================*/
static int scene_AddShape(SCENE *scene, size_t *capacity, const SHAPE *shape) {
    
    // Add a new shape to the scene
    if (scene->nshapes >= *capacity) {
        // Shape list is not initialized
        size_t size;
        // Check if we can grow
        if (*capacity * 2 < 0) {
#ifdef DEBUG
            fprintf(stderr, "scene_AddShape failed: Too many shapes\n");
#endif
            return FAILURE;
        }
        
        // Grow the shape list
        *capacity *= 2;
        size = *capacity * sizeof(SHAPE);
        SHAPE *new = (SHAPE *)realloc(scene->shapes, size);
        // Check for memory errors
        if (!new) {
#ifdef DEBUG
            fprintf(stderr, "scene_AddShape failed: Out of memory\n");
#endif
            return FAILURE;
        }
        scene->shapes = new;
    }
    
    // No problems - copy shape into scene
    memcpy(&scene->shapes[scene->nshapes++], shape, sizeof(SHAPE));
    return SUCCESS;
}

/*============================================================*
 * Validation
 *============================================================*/
static int scene_Validate(const SCENE *scene) {
    
    // Use this function to validate the entire scene after it has been
    // read from the file.
    
    // Validate view vector
    if (vector_IsZero(&scene->view)) {
#ifdef DEBUG
        fprintf(stderr, "scene_Validate failed: View vector is zero\n");
#endif
        return FAILURE;
    }
    
    // Validate up vector
    if (vector_IsZero(&scene->up)) {
#ifdef DEBUG
        fprintf(stderr, "scene_Validate failed: Up vector is zero\n");
#endif
        return FAILURE;
    } else if (vector_IsParalell(&scene->view, &scene->up)) {
#ifdef DEBUG
        fprintf(stderr, "scene_Validate failed: Up vector is paralell to view vector\n");
#endif
        return FAILURE;
    }
    
    // Validate FOV
    if (scene->fov <= 0.0 || scene->fov >= 180.0) {
#ifdef DEBUG
        fprintf(stderr, "scene_Validate failed: Invalid FOV %lf\n", scene->fov);
#endif
        return FAILURE;
    }
    
    // Validate dimensions
    if (scene->width <= 0 || scene->height <= 0) {
#ifdef DEBUG
        fprintf(stderr, "scene_Validate failed: Nonpositive image dimension discovered\n");
#endif
        return FAILURE;
    }
    
    // Correct
    return SUCCESS;
}

/*============================================================*
 * Check missing keywords
 *============================================================*/
static const char *scene_GetFlagName(int flags) {
    
    // Get the name of the first flag present in
    // the flags bitset.
    
    if (flags & FLAG_EYE) {
        return "eye";
    } else if (flags & FLAG_VIEW) {
        return "viewdir";
    } else if (flags & FLAG_UP) {
        return "updir";
    } else if (flags & FLAG_FOV) {
        return "fovv";
    } else if (flags & FLAG_SIZE) {
        return "imsize";
    } else if (flags & FLAG_BACKGROUND) {
        return "bkgcolor";
    } else if (flags & FLAG_MATERIEL) {
        return "mtlcolor";
    } else {
        return NULL;
    }
}

/*============================================================*
 * Whitespace checking
 *============================================================*/
static int string_IsAllWhitespace(const char *str) {
    int i = 0;
    while (str[i]) {
        if (!isspace(str[i])) {
            return 0;
        }
    }
    return 1;
}

/*============================================================*
 * Main decoder function
 *============================================================*/
int scene_Decode(SCENE *scene, const char *filename) {

    // Load a scene from the file
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Cannot open %s\n", filename);
#endif
        return -1;
    }

    // Set up reading buffer
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    char keyword[11];
    size_t delta;
    
    // Decoder state initialization
    RGB materiel;
    int flags = 0;
    int line = 0;
    size_t capacity = 1;
    
    SHAPE shape;
    shape.shape = 0;
    
    // Reading variables
    char *where;
    
    // Read every line of input file
    while (line++, fgets(buf, BUF_SIZE, file)) {
        
        // Skip comments or empty lines
        if (buf[0] == '#' || string_IsAllWhitespace(buf)) {
            continue;
        }
        
        // Read the keyword
        if (sscanf(buf, "%10c%n", keyword, &delta) != 1) {
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Parse error on line %d \"%s\"\n", line, buf);
#endif
            fclose(file);
            return FAILURE;
        }
        // Read from here
        where = buf + delta;
        
        // Parse the keyword arguments
        const char *format;
        void *args[8];
        int number;
        shape.shape = SHAPE_NONE;
        
        if (!strcmp(keyword, "eye")) {
            flags |= FLAG_EYE;
            format = "%lf %lf %lf%n";
            args[0] = &scene->eye.x;
            args[1] = &scene->eye.y;
            args[2] = &scene->eye.z;
            args[3] = &delta;
            number = 3;
            
        } else if (!strcmp(keyword, "viewdir")) {
            flags |= FLAG_VIEW;
            format = "%lf %lf %lf%n";
            args[0] = &scene->view.x;
            args[1] = &scene->view.y;
            args[2] = &scene->view.z;
            args[3] = &delta;
            number = 3;
            
        } else if (!strcmp(keyword, "updir")) {
            flags |= FLAG_UP;
            format = "%lf %lf %lf%n";
            args[0] = &scene->up.x;
            args[1] = &scene->up.y;
            args[2] = &scene->up.z;
            args[3] = &delta;
            number = 3;
            
        } else if (!strcmp(keyword, "fovv")) {
            flags |= FLAG_FOV;
            format = "%lf%n";
            args[0] = &scene->fov;
            args[1] = &delta;
            number = 1;

        } else if (!strcmp(keyword, "imsize")) {
            flags |= FLAG_SIZE;
            format = "%d %d%n";
            args[0] = &scene->width;
            args[1] = &scene->height;
            args[2] = &delta;
            number = 2;
      
        } else if (!strcmp(keyword, "bkgcolor")) {
            flags |= FLAG_BACKGROUND;
            format = "%hhu %hhu %hhu%n";
            args[0] = &scene->background.r;
            args[1] = &scene->background.g;
            args[2] = &scene->background.b;
            args[3] = &delta;
            number = 3;
            
        } else if (!strcmp(keyword, "mtlcolor")) {
            flags |= FLAG_MATERIEL;
            format = "%hhu %hhu %hhu%n";
            args[0] = &materiel.r;
            args[1] = &materiel.g;
            args[2] = &materiel.b;
            args[3] = &delta;
            number = 3;

        } else if (!strcmp(keyword, "sphere")) {
            shape.shape = SHAPE_SPHERE;
            format = "%lf %lf %lf %lf%n";
            args[0] = &shape.center.x;
            args[1] = &shape.center.y;
            args[2] = &shape.center.z;
            args[3] = &shape.data.radius;
            args[4] = &delta;
            number = 4;
            
        } else if (!strcmp(keyword, "ellipsoid")) {
            shape.shape = SHAPE_ELLIPSOID;
            format = "%lf %lf %lf %lf %lf %lf%n";
            args[0] = &shape.center.x;
            args[1] = &shape.center.y;
            args[2] = &shape.center.z;
            args[3] = &shape.data.dimension.x;
            args[4] = &shape.data.dimension.y;
            args[5] = &shape.data.dimension.z;
            args[6] = &delta;
            number = 6;
            
        } else {
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Invalid keyword \"%s\"\n", keyword);
#endif
            fclose(file);
            return FAILURE;
        }
        
        // Unload the data (ok to have too many pointers)
        if (sscanf(where, format, &args[0], &args[1], &args[2], &args[3], &args[4], &args[5], &args[6], &args[7]) != number) {
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Error parsing %s keyword on line %d\n", keyword, line);
#endif
            return FAILURE;
        }
        where += delta;
        if (!string_IsAllWhitespace(where)) {
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Extra information on line %d\n", line);
#endif
            return FAILURE;
        }
        
        // Install shape if found
        if (shape.shape != SHAPE_NONE) {
            // Install shape into scene
            if (!(flags & FLAG_MATERIEL)) {
#ifdef DEBUG
                fprintf(stderr, "scene_Decode failed: Discovered shape before materiel color\n");
#endif
                return FAILURE;
            }
            memcpy(&shape.color, &materiel, sizeof(RGB));
            if (scene_AddShape(scene, &capacity, &shape) == FAILURE) {
#ifdef DEBUG
                fprintf(stderr, "scene_Decode failed: Failed to add shape\n");
#endif
                return FAILURE;
            }
        }
    }
    
    // Clean up
    fclose(file);
    
    // Check for missing keywords
    const char *missing = scene_GetFlagName((~flags) & FLAG_ALL);
    if (missing) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Missing %s keyword\n", missing);
#endif
        return FAILURE;
     }
    
    // Validate the scene
    if (!scene_Validate(scene)) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Invalid scene\n");
#endif
        return FAILURE;
    }
    
    // Shrink arraylist to conserve memory
    SHAPE *new = (SHAPE *)realloc(scene->shapes, sizeof(SHAPE) * scene->nshapes);
    if (new) {
        scene->shapes = new;
    }

    // Successfully parsed the whole scene
    return 0;
}

/*============================================================*
 * Scene destructor
 *============================================================*/
void scene_Destroy(SCENE *scene) {
    if (scene->shapes) {
        free(scene->shapes);
    }
}

/*============================================================*/
