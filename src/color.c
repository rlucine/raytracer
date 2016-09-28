/**********************************************************//**
 * @file color.c
 * @brief Implementation of color definitions
 * @author Alec Shinomiya
 **************************************************************/

// This project
#include "color.h"
#include "vector.h"

/*============================================================*
 * Convert RGB to COLOR
 *============================================================*/
void rgb_ToColor(COLOR *color, const RGB *rgb) {
    color->x = (double)rgb->r / RGB_MAX;
    color->y = (double)rgb->g / RGB_MAX;
    color->z = (double)rgb->b / RGB_MAX;
}

void color_ToRgb(RGB *rgb, const COLOR *color) {
    rgb->r = color->x * RGB_MAX;
    rgb->g = color->y * RGB_MAX;
    rgb->b = color->z * RGB_MAX;
}

/*============================================================*/
