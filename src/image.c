/**********************************************************//**
 * @file image.c
 * @brief Implementation file for image data
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, size_t
#include <stdio.h>      // fopen, fclose, fprintf, getline ...
#include <limits.h>     // USHRT_MAX
#include <sys/stat.h>   // stat, struct stat
#include <string.h>     // strcpy
#include <assert.h>     // assert

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "color.h"      // COLOR, RGB ...

// Debugging modules
#ifdef TRACE
#include "tracemalloc.h"
#endif

/*============================================================*
 * Allocating IMAGE
 *============================================================*/
int image_Create(IMAGE *image, int width, int height) {
    
    // Error check the dimensions
    if (width <= 0 || width > MAX_DIMENSION) {
#ifdef VERBOSE
        fprintf(stderr, "image_Create failed: Invalid width %d\n", width);
#endif
        return FAILURE;

    } else if (height <= 0 || height > MAX_DIMENSION) {
#ifdef VERBOSE
        fprintf(stderr, "image_Create failed: Invalid height %d\n", height);
#endif
        return FAILURE;
    }
    
    // Allocate the image data chunk
    size_t size = sizeof(RGB)*width*height;
    if (size < 0 || size > INT_MAX) {
#ifdef VERBOSE
        fprintf(stderr, "image_Create failed: %d by %d image is too large\n", width, height);
#endif
        return FAILURE;
    }
    RGB *data = (RGB *)malloc(size);
    if (!data) {
#ifdef VERBOSE
        fprintf(stderr, "image_Create failed: Out of heap space\n");
#endif
        return FAILURE;
    }
    
    // Set up the struct
    image->width = width;
    image->height = height;
    image->data = data;
    return SUCCESS;
}

/*============================================================*
 * Reading pixels
 *============================================================*/
const RGB *image_GetPixel(const IMAGE *image, int x, int y) {
    
    // Error check the index
    if (x < 0 || x >= image->width) {
#ifdef VERBOSE
        fprintf(stderr, "image_GetColor failed: Invalid x coordinate\n");
#endif
        return NULL;
    }
    if (y < 0 || y >= image->height) {
#ifdef VERBOSE
        fprintf(stderr, "image_GetColor failed: Invalid y coordinate\n");
#endif
        return NULL;
    }
    
    // Output the color
    return &image->data[y*image->width + x];
}

/*============================================================*
 * Modifying pixels
 *============================================================*/
int image_SetPixel(IMAGE *image, int x, int y, const RGB *color) {
    
    // Error check the index
    if (x < 0 || x >= image->width) {
#ifdef VERBOSE
        fprintf(stderr, "image_SetColor failed: Invalid x coordinate\n");
#endif
        return FAILURE;
    }
    if (y < 0 || y >= image->height) {
#ifdef VERBOSE
        fprintf(stderr, "image_SetColor failed: Invalid y coordinate\n");
#endif
        return FAILURE;
    }
    
    // Set the color
    int index = y*image->width + x;
    RGB *where = &image->data[index];
    where->r = color->r;
    where->g = color->g;
    where->b = color->b;
    
    // Done
    return SUCCESS;
}

/*============================================================*
 * Texturing
 *============================================================*/
int image_GetTexture(const TEXTURE *texture, const TEXCOORD *coord, COLOR *color, int flags) {
    
    // Mapping to pixels
    int x = (int)(coord->x * texture->width);
    int y = (int)(coord->y * texture->height);
    double dx = (coord->x * texture->width) - x;
    double dy = (coord->y * texture->height) - y;
    
    // Error check coordinates
    if (coord->x < 0.0 || coord->x > 1.0) {
#ifdef VERBOSE
        fprintf(stderr, "image_GetTexture failed: Invalud x coordinate.\n");
#endif
        return FAILURE;
    }
    if (coord->y < 0.0 || coord->y > 1.0) {
#ifdef VERBOSE
        fprintf(stderr, "image_GetTexture failed: Invalud y coordinate.\n");
#endif
        return FAILURE;
    }
    
    // Collect colors from image
    const RGB *rgb;
    if (flags & TEXTURE_INTERPOLATE) {
        // Zero out the color
        COLOR temp;
        color->x = color->y = color->z = 0.0;
        
        // Interpolate all components of nearby colors
        // Skip colors that don't exist - will this cause
        // the edges to become darker?
        if ((rgb = image_GetPixel(texture, x, y)) != NULL) {
            rgb_ToColor(&temp, rgb);
            vector_Multiply(&temp, &temp, (1-dx)*(1-dy));
            vector_Add(color, color, &temp);
        }
        if ((rgb = image_GetPixel(texture, x+1, y)) != NULL) {
            rgb_ToColor(&temp, rgb);
            vector_Multiply(&temp, &temp, dx*(1-dy));
            vector_Add(color, color, &temp);
        }
        if ((rgb = image_GetPixel(texture, x, y+1)) != NULL) {
            rgb_ToColor(&temp, rgb);
            vector_Multiply(&temp, &temp, (1-dx)*dy);
            vector_Add(color, color, &temp);
        }
        if ((rgb = image_GetPixel(texture, x+1, y+1)) != NULL) {
            rgb_ToColor(&temp, rgb);
            vector_Multiply(&temp, &temp, dx*dy);
            vector_Add(color, color, &temp);
        }
        
    } else {
        // Just do nearest neighbor
        if ((rgb = image_GetPixel(texture, x, y)) != NULL) {
            rgb_ToColor(color, rgb);
        }
    }
    return SUCCESS;
}

/*============================================================*
 * Getters
 *============================================================*/
int image_GetWidth(const IMAGE *image) {
    return image->width;
}

int image_GetHeight(const IMAGE *image) {
    return image->height;
}

/*============================================================*
 * Destroy image
 *============================================================*/
void image_Destroy(IMAGE *image) {
    if (image->data) {
        free(image->data);
        image->data = NULL;
    }
    return;
}

/*============================================================*/
