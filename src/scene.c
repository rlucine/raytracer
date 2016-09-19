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

// This project
#include "rgb.h"
#include "vector.h"
#include "shape.h"
#include "scene.h"
#include "tracemalloc.h"

/*============================================================*
 * Whitespace checking
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
 * Add shapes to scene
 * scene: The scene to mutate
 * capacity: Pointer to the capacity of the scene->shapes
 * arraylist. This can be mutated!
 * shape: The shape to copy into the arraylist.
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
 * Decode scene data
 * Each field should only be specified once
 *============================================================*/
static int scene_DecodeData(SCENE *scene, const char *encoded) {
    
    // Decode integers to these
    double r, g, b, width, height;
    
    // Parse the line
    int nread;
    
    /*---------------------------------------------*
     * eye
     *---------------------------------------------*/
    if (sscanf(encoded, "eye %lf %lf %lf%n", &scene->eye.x, &scene->eye.y, &scene->eye.z, &nread) == 3) {
        // Found valid eye
        if (scene->flags & FLAG_EYE) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of eye\n");
#endif
            return FAILURE;
        }
        scene->flags |= FLAG_EYE;
        
    /*---------------------------------------------*
     * view
     *---------------------------------------------*/
    } else if (sscanf(encoded, "viewdir %lf %lf %lf%n", &scene->view.x, &scene->view.y, &scene->view.z, &nread) == 3) {
        // Found valid view definition
        if (scene->flags & FLAG_VIEW) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of viewdir\n");
#endif
            return FAILURE;
        }
        if (vector_IsZero(&scene->view)) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Null view vector\n");
#endif
            return FAILURE;
        }
        if ((scene->flags & FLAG_UP) && vector_IsParalell(&scene->up, &scene->view)) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Up vector parallel to view vector\n");
#endif
            return FAILURE;
        }
        scene->flags |= FLAG_VIEW;
    
    /*---------------------------------------------*
     * up
     *---------------------------------------------*/
    } else if (sscanf(encoded, "updir %lf %lf %lf%n", &scene->up.x, &scene->up.y, &scene->up.z, &nread) == 3) {
        // Found valid view definition
        if (scene->flags & FLAG_UP) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of updir\n");
#endif
            return FAILURE;
        }
        if (vector_IsZero(&scene->up)) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Null up vector\n");
#endif
            return FAILURE;
        }
        if ((scene->flags & FLAG_VIEW) && vector_IsParalell(&scene->up, &scene->view)) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Up vector parallel to view vector\n");
#endif
            return FAILURE;
        }
        scene->flags |= FLAG_UP;
        
    /*---------------------------------------------*
     * fovv
     *---------------------------------------------*/
    } else if (sscanf(encoded, "fovv %lf%n", &scene->fov, &nread) == 1) {
        // Found valid view definition
        if (scene->flags & FLAG_FOV) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of fovv\n");
#endif
            return FAILURE;
        }
        if (scene->fov <= MIN_FOV || scene->fov >= MAX_FOV) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Impossible fov %lf\n", scene->fov);
#endif
            return FAILURE;
        }
        scene->flags |= FLAG_FOV;
     
    /*---------------------------------------------*
     * imsize
     *---------------------------------------------*/
    } else if (sscanf(encoded, "imsize %lf %lf%n", &width, &height, &nread) == 2) {
        // Found valid view definition
        if (scene->flags & FLAG_SIZE) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of imsize\n");
#endif
            return FAILURE;
        }
        if ((int)width <= 0 || (int)height <= 0) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Invalid image size (%lf, %lf)\n", width, height);
#endif
            return FAILURE;
        }
        // Allow for parsing of doubles
        scene->width = (int)width;
        scene->height = (int)height;
        scene->flags |= FLAG_SIZE;
     
    /*---------------------------------------------*
     * bkgcolor
     *---------------------------------------------*/
    } else if (sscanf(encoded, "bkgcolor %lf %lf %lf%n", &r, &g, &b, &nread) == 3) {
        // Found valid view definition
        if (scene->flags & FLAG_BACKGROUND) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Multiple definition of bkgcolor\n");
#endif
            return FAILURE;
        }
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeData failed: Invalid background color (%lf, %lf, %lf)\n", r, g, b);
#endif
            return FAILURE;
        }
        // Allow for parsing of doubles
        scene->background.r = (char)r;
        scene->background.g = (unsigned char)g;
        scene->background.b = (unsigned char)b;
        scene->flags |= FLAG_BACKGROUND;
        
    } else {
        // Don't print an error message
        return NO_MATCH;
    }
    
    // Extra line check
    if (!string_IsAllWhitespace(encoded + nread)) {
#ifdef DEBUG
        fprintf(stderr, "scene_DecodeData failed: Trailing \"%s\" on end of line\n", encoded + nread);
#endif
        return FAILURE;
    }
    
    // Succeeded
    return SUCCESS;
}

/*============================================================*
 * Decode material data
 * Material tokens will appear in the shape stream
 * Each one may appear multiple times
 *============================================================*/
static int scene_DecodeMaterial(MATERIAL *material, const char *encoded) {
    
    // Decode into this so we don't corrupt state
    double r, g, b;
    
    // Parse the line
    int nread;
    
    /*---------------------------------------------*
     * mtlcolor
     *---------------------------------------------*/
    if (sscanf(encoded, "mtlcolor %lf %lf %lf%n", &r, &g, &b, &nread) == 3) {
        // Found material color
        if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeMaterial failed: Invalid color (%lf, %lf, %lf)\n", r, g, b);
#endif
            return FAILURE;
        }
        // Cast color to correct type
        material->color.r = (unsigned char)r;
        material->color.g = (unsigned char)g;
        material->color.b = (unsigned char)b;
        
    } else {
        // Don't print an error message
        return NO_MATCH;
    }
    
    // Extra line check
    if (!string_IsAllWhitespace(encoded + nread)) {
#ifdef DEBUG
        fprintf(stderr, "scene_DecodeMaterial failed: Trailing \"%s\" on end of line\n", encoded + nread);
#endif
        return FAILURE;
    }
    
    // Succeeded
    return SUCCESS;
}

/*============================================================*
 * Decode shape data from encoded and put it in shape
 * Any number of shapes may appear
 *============================================================*/
static int scene_DecodeShape(SHAPE *shape, const char *encoded) {
    
    // Shape data packages
    SPHERE sphere;
    ELLIPSOID ellipsoid;
    
    // Parse the line
    int nread;
    
    /*---------------------------------------------*
     * sphere
     *---------------------------------------------*/
    if (sscanf(encoded, "sphere %lf %lf %lf %lf%n", &sphere.center.x, &sphere.center.y, &sphere.center.z, &sphere.radius, &nread) == 4) {
        // Found a valid sphere - now check its properties
        if (sphere.radius <= 0.0) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeShape failed: Invalid sphere radius %lf\n", sphere.radius);
#endif
            return FAILURE;
        }
        shape->shape = SHAPE_SPHERE;
        
    /*---------------------------------------------*
     * ellipsoid
     *---------------------------------------------*/
    } else if (sscanf(encoded, "ellipsoid %lf %lf %lf %lf %lf %lf%n", &ellipsoid.center.x, &ellipsoid.center.y, &ellipsoid.center.z, &ellipsoid.dimension.x, &ellipsoid.dimension.y, &ellipsoid.dimension.z, &nread) == 6) {
        // Found an ellipsoid
        if (ellipsoid.dimension.x <= 0.0 || ellipsoid.dimension.y <= 0.0 || ellipsoid.dimension.z <= 0.0) {
#ifdef DEBUG
            fprintf(stderr, "scene_DecodeShape failed: Invalid ellipsoid dimension (%lf, %lf, %lf)\n", ellipsoid.dimension.x, ellipsoid.dimension.y, ellipsoid.dimension.z);
#endif
            return FAILURE;
        }
        shape->shape = SHAPE_ELLIPSOID;
        
    } else {
        // Don't print an error message
        return NO_MATCH;
    }
    
    // Extra line check
    if (!string_IsAllWhitespace(encoded + nread)) {
#ifdef DEBUG
        fprintf(stderr, "scene_DecodeShape failed: Trailing \"%s\" on end of line\n", encoded + nread);
#endif
        return FAILURE;
    }
    
    // Install shape
    size_t size;
    switch (shape->shape) {
    case SHAPE_SPHERE:
        size = sizeof(SPHERE);
        shape->data = (void *)&sphere;
        break;
        
    case SHAPE_ELLIPSOID:
        size = sizeof(ELLIPSOID);
        shape->data = (void *)&ellipsoid;
        break;
        
    default:
    case SHAPE_NONE:
        // Should never get here
#ifdef DEBUG
        fprintf(stderr, "scene_DecodeShape failed: Invalid shape type %d\n", shape->shape);
#endif
        return FAILURE;
    }
    
    // Perform allocation - initially shape->data holds a stack variable
    // Make shape->data hold an equal heap variable
    void *data = malloc(size);
    if (!data) {
#ifdef DEBUG
        fprintf(stderr, "scene_DecodeShape failed: Out of memory\n");
#endif
        return FAILURE;
    }
    memcpy(data, shape->data, size);
    shape->data = data;
    
    // Successfully decoded shape
    return SUCCESS;
}

/*============================================================*
 * Main decoder function
 *============================================================*/
int scene_Decode(SCENE *scene, const char *filename) {

    // File reading initialize
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Cannot open %s\n", filename);
#endif
        return FAILURE;
    }

    // Set up reading buffer
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    
    // Decoder state initialize
    scene->flags = 0;
    SHAPE shape;
    int line = 0;
    
    // Initialize shapes araylist
    size_t capacity = 1;
    scene->nshapes = 0;
    scene->shapes = (SHAPE *)malloc(sizeof(SHAPE) * capacity);
    if (!scene->shapes) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Out of memory\n");
#endif
        return FAILURE;
    }
    
    // Main parser loop
    int result;
    while (line++, fgets(buf, BUF_SIZE, file)) {
        
        // Skip comments or empty lines
        if (buf[0] == '#' || string_IsAllWhitespace(buf)) {
            continue;
        }
        
        // Try parsing scene data
        switch (result = scene_DecodeData(scene, buf)) {
        case SUCCESS:
            continue;
            
        case FAILURE:
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", line);
#endif
            fclose(file);
            return FAILURE;
        
        case NO_MATCH:
        default:
            break;
        }
        
        // Try parsing material data
        switch (result = scene_DecodeMaterial(&shape.material, buf)) {
        case SUCCESS:
            // Found material property
            scene->flags |= FLAG_MATERIAL;
            continue;
            
        case FAILURE:
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", line);
#endif
            fclose(file);
            return FAILURE;
        
        case NO_MATCH:
        default:
            break;
        }
        
        // Try parsing shape data
        switch (result = scene_DecodeShape(&shape, buf)) {
        case SUCCESS:
            // Can't define a shape if material not defined!
            if (!(scene->flags & FLAG_MATERIAL)) {
#ifdef DEBUG
                fprintf(stderr, "scene_Decode failed: Discovered shape before material color\n");
#endif
                fclose(file);
                return FAILURE;
            }
            if (scene_AddShape(scene, &capacity, &shape) == FAILURE) {
#ifdef DEBUG
                fprintf(stderr, "scene_Decode failed: Failed to add shape\n");
#endif
                fclose(file);
                return FAILURE;
            }
            continue;
            
        case FAILURE:
#ifdef DEBUG
            fprintf(stderr, "scene_Decode failed: Parse error on line %d\n", line);
#endif
            fclose(file);
            return FAILURE;
        
        case NO_MATCH:
        default:
            break;
        }
        
        // No match whatsoever
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Unknown line %d \"%s\"\n", line, buf);
#endif
        fclose(file);
        return FAILURE;
    }
    
    // Clean up
    fclose(file);
    
    // Check missing flags
    int missing_flags = ~scene->flags;
    const char *missing;
    if (missing_flags & FLAG_EYE) {
        missing = "eye";
    } else if (missing_flags & FLAG_VIEW) {
        missing = "viewdir";
    } else if (missing_flags & FLAG_UP) {
        missing = "updir";
    } else if (missing_flags & FLAG_FOV) {
        missing = "fovv";
    } else if (missing_flags & FLAG_SIZE) {
        missing = "imsize";
    } else if (missing_flags & FLAG_BACKGROUND) {
        missing = "bkgcolor";
    } else {
        missing = NULL;
    }
    if (missing) {
#ifdef DEBUG
        fprintf(stderr, "scene_Decode failed: Missing %s definition\n", missing);
#endif
        return FAILURE;
    }
    
    // Shrink arraylist to conserve memory
    SHAPE *new = (SHAPE *)malloc(sizeof(SHAPE) * scene->nshapes);
    if (new) {
        memcpy(new, scene->shapes, sizeof(SHAPE) * scene->nshapes);
        free(scene->shapes);
        scene->shapes = new;
    }

    // Successfully parsed the whole scene
    return SUCCESS;
}

/*============================================================*
 * Scene getters
 *============================================================*/
const POINT *scene_GetEyePosition(const SCENE *scene) {
    return &scene->eye;
}

const VECTOR *scene_GetViewDirection(const SCENE *scene) {
    return &scene->view;
}

const VECTOR *scene_GetUpDirection(const SCENE *scene) {
    return &scene->up;
}

double scene_GetFieldOfView(const SCENE *scene) {
    return scene->fov;
}

int scene_GetWidth(const SCENE *scene) {
    return scene->width;
}

int scene_GetHeight(const SCENE *scene) {
    return scene->height;
}

const RGB *scene_GetBackgroundColor(const SCENE *scene) {
    return &scene->background;
}

int scene_GetNumberOfShapes(const SCENE *scene) {
    return scene->nshapes;
}

const SHAPE *scene_GetShape(const SCENE *scene, int index) {
    return &scene->shapes[index];
}

/*============================================================*
 * Scene destructor
 *============================================================*/
void scene_Destroy(SCENE *scene) {
    int i;
    
    // Free all the allocated shape data (SHAPE not responsible)
    for (i = 0; i < scene->nshapes; i++) {
       free(scene->shapes[i].data);
    }
    
    // Free allocated array
    if (scene->shapes) {
       free(scene->shapes);
    }
}

/*============================================================*/
