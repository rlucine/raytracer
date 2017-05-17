/**********************************************************//**
 * @file color.h
 * @brief Header file for color definitions
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _COLOR_H_
#define _COLOR_H_

// This project
#include "vector.h"     // VECTOR

//*************************************************************
#define RGB_MAX 255     ///< The maximum value for an RGB color.
#define RGB_MIN 0       ///< The minimum value for an RGB color.
#define COLOR_MAX 1.0   ///< The maximum value for a COLOR component.
#define COLOR_MIN 0.0   ///< The minimum value for a COLOR component.

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
static inline void color_ToRgb(RGB *rgb, const COLOR *color) {
    rgb->r = color->x * RGB_MAX;
    rgb->g = color->y * RGB_MAX;
    rgb->b = color->z * RGB_MAX;
}

/**********************************************************//**
 * @brief Convert RGB to COLOR struct
 * @param color: The color struct to fill with data.
 * @param rgb: The rgb struct to read.
 **************************************************************/
static inline void rgb_ToColor(COLOR *color, const RGB *rgb) {
    color->x = (float)rgb->r / RGB_MAX;
    color->y = (float)rgb->g / RGB_MAX;
    color->z = (float)rgb->b / RGB_MAX;
}

/**********************************************************//**
 * @brief Clamp the color in the proper range from COLOR_MIN
 * to COLOR_MAX.
 * @param color: The color to clamp.
 **************************************************************/
extern void color_Clamp(COLOR *color);

/*============================================================*/
#endif // _COLOR_H_
