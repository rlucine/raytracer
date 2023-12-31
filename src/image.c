/**********************************************************//**
 * @file image.c
 * @brief Implementation file for image data
 * @author Rena Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <stdlib.h>     // malloc, free

// This project
#include "image.h"      // IMAGE
#include "color.h"      // RGB

// Debugging modules
#include "debug.h"      // eprintf, assert

/*============================================================*
 * Allocating IMAGE
 *============================================================*/
bool image_Create(IMAGE *image, int width, int height) {
    // Error check the dimensions
    if (width <= 0 || width > MAX_DIMENSION) {
        eprintf("Invalid width %d\n", width);
        return false;

    } else if (height <= 0 || height > MAX_DIMENSION) {
        eprintf("Invalid height %d\n", height);
        return false;
    }
    
    // Allocate the image data chunk
    RGB *data = (RGB *)malloc(sizeof(RGB)*width*height);
    if (!data) {
        eprintf("Out of heap space\n");
        return false;
    }
    
    // Set up the struct
    image->width = width;
    image->height = height;
    image->data = data;
    return true;
}

/*============================================================*
 * Reading pixels
 *============================================================*/
const RGB *image_GetPixel(const IMAGE *image, int x, int y) {
    // Error check the index
    if (x < 0 || x >= image->width) {
        eprintf("Invalid x coordinate\n");
        return NULL;
    }
    if (y < 0 || y >= image->height) {
        eprintf("Invalid y coordinate\n");
        return NULL;
    }
    
    // Output the color
    return &image->data[y*image->width + x];
}

/*============================================================*
 * Modifying pixels
 *============================================================*/
bool image_SetPixel(IMAGE *image, int x, int y, const RGB *color) {
    // Error check the index
    if (x < 0 || x >= image->width) {
        eprintf("Invalid x coordinate\n");
        return false;
    }
    if (y < 0 || y >= image->height) {
        eprintf("Invalid y coordinate\n");
        return false;
    }
    
    // Set the color
    int index = y*image->width + x;
    RGB *where = &image->data[index];
    where->r = color->r;
    where->g = color->g;
    where->b = color->b;
    return true;
}

/*============================================================*
 * Texturing
 *============================================================*/
bool image_GetTexture(const TEXTURE *texture, const TEXCOORD *coord, COLOR *color) {
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
        return true;
    }
    return false;
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
