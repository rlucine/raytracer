/**********************************************************//**
 * @file image.h
 * @brief Header file for image buffers
 * @author Rena Shinomiya
 **************************************************************/

#ifndef _IMAGE_H_
#define _IMAGE_H_

// Standard library
#include <stdbool.h>    // bool

// This project
#include "vector.h"     // VECTOR
#include "color.h"      // COLOR

//*************************************************************
#define MAX_DIMENSION 32767     ///< The largest image dimension.

/**********************************************************//**
 * @struct IMAGE
 * @brief Struct for uncompressed RGB image data
 **************************************************************/
typedef struct {
    int width;          ///< Width of the image
    int height;         ///< Height of the image
    RGB *data;          ///< Do not access this variable
} IMAGE;

/**********************************************************//**
 * @typedef TEXTURE
 * @brief Image format for texture maps
 **************************************************************/
typedef IMAGE TEXTURE;

/**********************************************************//**
 * @typedef TEXCOORD
 * @brief Struct for texture coordinates.
 **************************************************************/
typedef VECTOR TEXCOORD;

/**********************************************************//**
 * @brief Allocate space for an image
 * @param image: Pointer to an uninitialized IMAGE struct. You
 * need to use image_Destroy on this parameter if the function
 * is successful. Don't pass a image which has already been
 * initialized because a memory leak wil occur.
 * @param width: The width of the image
 * @param height: The height of the image
 * @return Whether the creation succeeded.
 **************************************************************/
extern bool image_Create(IMAGE *image, int width, int height);

/**********************************************************//**
 * @brief Get the RGB color located at the given coordinate
 * in the image data.
 * @param image: The image data to read
 * @param x: The x coordinate from 0 to image->width - 1
 * @param y: The y coordinate from 0 to image->height - 1
 * @return Pointer to an RGB struct containing the data
 **************************************************************/
extern const RGB *image_GetPixel(const IMAGE *image, int x, int y);

/**********************************************************//**
 * @brief Set the RGB color at the given location in an IMAGE.
 * @param image: The image data to mutate
 * @param x: The x coordinate from 0 to image->width - 1
 * @param y: The y coordinate from 0 to image->height - 1
 * @param color: The RGB color to set
 * @return Whether the access succeeded.
 **************************************************************/
extern bool image_SetPixel(IMAGE *image, int x, int y, const RGB *color);

/**********************************************************//**
 * @brief Treating the image as a texture, get the texture
 * color given texture coordinates.
 * @param texture: The image data to read
 * @param coord: The texture coordinate to look up.
 * @param color: Pointer to buffer to write the color in.
 * @return Whether the access succeeded.
 **************************************************************/
extern bool image_GetTexture(const TEXTURE *texture, const TEXCOORD *coord, COLOR *color);

/**********************************************************//**
 * @brief Get the width of the image data
 * @param image: The image data to read
 * @return The width of the image
 **************************************************************/
static inline int image_GetWidth(const IMAGE *image) {
    return image->width;
}

/**********************************************************//**
 * @brief Get the height of the image data
 * @param image: The image data to read
 * @return The height of the image
 **************************************************************/
static inline int image_GetHeight(const IMAGE *image) {
    return image->height;
}

/**********************************************************//**
 * @brief Delete an image without memory leaks
 * @param image: The image data to destroy. This data will
 * become unusable after calling this function. You may use
 * image_Create once more to initialize it again.
 **************************************************************/
extern void image_Destroy(IMAGE *image);

/*============================================================*/
#endif // _IMAGE_H_