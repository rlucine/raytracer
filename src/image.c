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
#include "debug.h"

/*============================================================*
 * Allocating IMAGE
 *============================================================*/
int image_Create(IMAGE *image, int width, int height) {
    
    // Error check the dimensions
    if (width <= 0 || width > MAX_DIMENSION) {
        errmsg("Invalid width %d\n", width);
        return FAILURE;

    } else if (height <= 0 || height > MAX_DIMENSION) {
        errmsg("Invalid height %d\n", height);
        return FAILURE;
    }
    
    // Allocate the image data chunk
    size_t size = sizeof(RGB)*width*height;
    if ((long)size < 0 || (long)size > INT_MAX) {
        errmsg("%d by %d image is too large\n", width, height);
        return FAILURE;
    }
    RGB *data = (RGB *)malloc(size);
    if (!data) {
        errmsg("Out of heap space\n");
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
        errmsg("Invalid x coordinate\n");
        return NULL;
    }
    if (y < 0 || y >= image->height) {
        errmsg("Invalid y coordinate\n");
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
        errmsg("Invalid x coordinate\n");
        return FAILURE;
    }
    if (y < 0 || y >= image->height) {
        errmsg("Invalid y coordinate\n");
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
int image_GetTexture(const TEXTURE *texture, const TEXCOORD *coord, COLOR *color) {
    
    // Mapping to pixels
    int x = (int)(coord->x * texture->width);
    int y = (int)(coord->y * texture->height);
    
    // Constrain location
    if (x < 0) {
        x = 0;
    } else if (x >= texture->width) {
        x = texture->width - 1;
    }
    if (y < 0) {
        y = 0;
    } else if (y >= texture->height) {
        y = texture->height - 1;
    }
    
    // Collect colors from image
    const RGB *rgb;
    if ((rgb = image_GetPixel(texture, x, y)) != NULL) {
        rgb_ToColor(color, rgb);
        return SUCCESS;
    }
    return FAILURE;
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
