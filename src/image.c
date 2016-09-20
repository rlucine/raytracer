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
#include "image.h"
#include "tracemalloc.h"

/*============================================================*
 * Constants
 *============================================================*/
// The maximum height or width of any image
static const int MAX_DIMENSION = USHRT_MAX;

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
    }
    return;
}

/*============================================================*/
