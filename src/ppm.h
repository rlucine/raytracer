/**********************************************************//**
 * @file ppm.h
 * @brief Header file for P3 PPM file parsing
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _PPM_H_
#define _PPM_H_

/*============================================================*
 * Constants
 *============================================================*/

/// @def SUCCESS
/// @brief Returned by PPM functions if the operation succeeded
#define SUCCESS 0

/// @def FAILURE
/// @brief Returned by PPM functions if the operation failed
#define FAILURE -1

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
 * @typedef PPM
 * @brief Struct for PPM image data. Use the functions defined
 * in this file instead of accessing members directly.
 **************************************************************/
typedef struct {
    unsigned short width;   ///< Width of the PPM image
    unsigned short height;  ///< Height of the PPM image
    RGB *data;              ///< Do not access this variable
} PPM;

/**********************************************************//**
 * @brief Allocate space for a PPM image containing garbage.
 * @param ppm: Pointer to an uninitialized PPM struct. You need
 * to use ppm_Destroy on this parameter if the function is
 * successful. Don't pass a ppm which has already been
 * initialized because a memory leak wil occur.
 * @param width: The width of the image
 * @param height: The height of the image
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_Create(PPM *ppm, int width, int height);

/**********************************************************//**
 * @brief Get the RGB color located at the given coordinate
 * in the PPM image data.
 * @param ppm: The image data to read
 * @param x: The x coordinate from 0 to ppm->width - 1
 * @param y: The y coordinate from 0 to ppm->height - 1
 * @return Pointer to an RGB struct containing the data
 **************************************************************/
extern const RGB *ppm_GetPixel(const PPM *ppm, int x, int y);

/**********************************************************//**
 * @brief Set the RGB color at the given location in a PPM
 * image.
 * @param ppm: The image data to mutate
 * @param x: The x coordinate from 0 to ppm->width - 1
 * @param y: The y coordinate from 0 to ppm->height - 1
 * @param color: The RGB color to set
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_SetPixel(PPM *ppm, int x, int y, const RGB *color);

/**********************************************************//**
 * @brief Get the width of the PPM image data
 * @param ppm: The image data to read
 * @return The width of the image
 **************************************************************/
extern int ppm_GetWidth(const PPM *ppm);

/**********************************************************//**
 * @brief Get the height of the PPM image data
 * @param ppm: The image data to read
 * @return The height of the image
 **************************************************************/
extern int ppm_GetHeight(const PPM *ppm);

/**********************************************************//**
 * @brief Encode a PPM structure as a P3 PPM image with
 * the given filename.
 * @param ppm: The initialized PPM data to encode
 * @param filename: The target filename
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_Encode(const PPM *ppm, const char *filename);

/**********************************************************//**
 * @brief Load a PPM image from a file
 * @param ppm: Pointer to an uninitialized PPM struct. You need
 * to use ppm_Destroy on this parameter if the function is
 * successful. Don't pass a ppm which has already been
 * initialized.
 * @param filename: The file to read.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_Decode(PPM *ppm, const char *filename);

/**********************************************************//**
 * @brief Delete a PPM image without memory leaks
 * @param ppm: The image data to destroy. This data will
 * become unusable after calling this function. You may use
 * ppm_Create once more to initialize it again.
 **************************************************************/
extern void ppm_Destroy(PPM *ppm);

/*============================================================*/
#endif // _PPM_H_