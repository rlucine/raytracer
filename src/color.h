/**********************************************************//**
 * @file color.h
 * @brief Header file for color definitions
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _COLOR_H_
#define _COLOR_H_

// This project
#include "vector.h"     // VECTOR

/**********************************************************//**
 * @def RGB_MAX
 * @brief The maximum value for an RGB color
 **************************************************************/
#define RGB_MAX 255

/**********************************************************//**
 * @def RGB_MIN
 * @brief The minimum value for an RGB color
 **************************************************************/
#define RGB_MIN 0

/**********************************************************//**
 * @def COLOR_MAX
 * @brief The maximum value for a COLOR component
 **************************************************************/
#define COLOR_MAX 1.0

/**********************************************************//**
 * @def COLOR_MIN
 * @brief The minimum value for a COLOR component
 **************************************************************/
#define COLOR_MIN 0.0

/**********************************************************//**
 * @struct RGB
 * @brief Struct for 24-bit RGB colors.
 **************************************************************/
typedef struct {
    unsigned char r;    ///< The red part of the color 0 - 255
    unsigned char g;    ///< The green part of the color 0 - 255
    unsigned char b;    ///< The blue part of the color 0 - 255
} RGB;

/**********************************************************//**
 * @typedef COLOR
 * @brief Struct for floating point color models. x refers to
 * red, y to blue, and z to green components, which should
 * be in the range [0, 1].
 **************************************************************/
typedef VECTOR COLOR;

/**********************************************************//**
 * @brief Convert COLOR to RGB struct.
 * @param rgb: The rgb struct to fill with data.
 * @param color: The color struct to read.
 **************************************************************/
extern void color_ToRgb(RGB *rgb, const COLOR *color);

/**********************************************************//**
 * @brief Convert RGB to COLOR struct
 * @param color: The color struct to fill with data.
 * @param rgb: The rgb struct to read.
 **************************************************************/
extern void rgb_ToColor(COLOR *color, const RGB *rgb);

/**********************************************************//**
 * @brief Clamp the color in the proper range from COLOR_MIN
 * to COLOR_MAX.
 * @param color: The color to clamp.
 **************************************************************/
extern void color_Clamp(COLOR *color);

/*============================================================*/
#endif // _COLOR_H_
