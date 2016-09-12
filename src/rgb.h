/**********************************************************//**
 * @file rgb.h
 * @brief Header file for RGB color system
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _RGB_H_
#define _RGB_H_

/**********************************************************//**
 * @typedef RGB
 * @brief Struct for RGB colors
 **************************************************************/
typedef struct {
    unsigned char r;    ///< The red part of the color 0 - 255
    unsigned char g;    ///< The green part of the color 0 - 255
    unsigned char b;    ///< The blue part of the color 0 - 255
} RGB;

/*============================================================*/
#endif // _RGB_H_
