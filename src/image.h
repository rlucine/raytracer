/**********************************************************//**
 * @file image.h
 * @brief Header file for image buffers
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _IMAGE_H_
#define _IMAGE_H_

// This project
#include "macro.h"

/**********************************************************//**
 * @typedef RGB
 * @brief Struct for RGB colors
 **************************************************************/
typedef struct {
    unsigned char r;    ///< The red part of the color 0 - 255
    unsigned char g;    ///< The green part of the color 0 - 255
    unsigned char b;    ///< The blue part of the color 0 - 255
} RGB;

/**********************************************************//**
 * @typedef IMAGE
 * @brief Struct for image data. Use the functions defined
 * in this file instead of accessing members directly.
 **************************************************************/
typedef struct {
    unsigned short width;   ///< Width of the image
    unsigned short height;  ///< Height of the image
    RGB *data;              ///< Do not access this variable
} IMAGE;

/**********************************************************//**
 * @brief Allocate space for an image containing garbage.
 * @param image: Pointer to an uninitialized IMAGE struct. You
 * need to use image_Destroy on this parameter if the function
 * is successful. Don't pass a image which has already been
 * initialized because a memory leak wil occur.
 * @param width: The width of the image
 * @param height: The height of the image
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int image_Create(IMAGE *image, int width, int height);

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
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int image_SetPixel(IMAGE *image, int x, int y, const RGB *color);

/**********************************************************//**
 * @brief Get the width of the image data
 * @param image: The image data to read
 * @return The width of the image
 **************************************************************/
extern int image_GetWidth(const IMAGE *image);

/**********************************************************//**
 * @brief Get the height of the image data
 * @param image: The image data to read
 * @return The height of the image
 **************************************************************/
extern int image_GetHeight(const IMAGE *image);

/**********************************************************//**
 * @brief Delete an image without memory leaks
 * @param image: The image data to destroy. This data will
 * become unusable after calling this function. You may use
 * image_Create once more to initialize it again.
 **************************************************************/
extern void image_Destroy(IMAGE *image);

/*============================================================*/
#endif // _IMAGE_H_