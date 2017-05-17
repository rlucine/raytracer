/**********************************************************//**
 * @file color.c
 * @brief Implementation of color definitions
 * @author Alec Shinomiya
 **************************************************************/

// This project
#include "color.h"  // COLOR

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
