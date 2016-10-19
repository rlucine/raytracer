/**********************************************************//**
 * @file color.c
 * @brief Implementation of color definitions
 * @author Alec Shinomiya
 **************************************************************/

// This project
#include "color.h"  // COLOR, RGB_MAX, RGB_MIN ...

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

/*============================================================*
 * Clamp colors
 *============================================================*/
void color_Clamp(COLOR *color) {
    if (color->x > COLOR_MAX) {
        color->x = COLOR_MAX;
    } else if (color->x < COLOR_MIN) {
        color->x = COLOR_MIN;
    }
    
    if (color->y > COLOR_MAX) {
        color->y = COLOR_MAX;
    } else if (color->y < COLOR_MIN) {
        color->y = COLOR_MIN;
    }
    
    if (color->z > COLOR_MAX) {
        color->z = COLOR_MAX;
    } else if (color->z < COLOR_MIN) {
        color->z = COLOR_MIN;
    }
}

/*============================================================*/
