/**********************************************************//**
 * @file ppm.h
 * @brief Header file for P3 PPM file parsing
 * @author Rena Shinomiya
 **************************************************************/

#ifndef _PPM_H_
#define _PPM_H_

// Standard library
#include <stdbool.h>    // bool

// This project
#include "image.h"  // IMAGE

//*************************************************************
/// The maximum color value for a PPM image
#define PPM_MAX_COLOR RGB_MAX

/// The maximum number of characters on a line of a PPM.
#define PPM_MAX_LINE 70

/**********************************************************//**
 * @brief Encode a PPM structure as a P3 PPM image with
 * the given filename.
 * @param ppm: The initialized PPM data to encode
 * @param filename: The target filename
 * @return Whether the image could be encoded.
 **************************************************************/
extern bool ppm_Encode(const IMAGE *ppm, const char *filename);

/**********************************************************//**
 * @brief Load a PPM image from a file
 * @param ppm: Pointer to an uninitialized PPM struct. You need
 * to use ppm_Destroy on this parameter if the function is
 * successful. Don't pass a ppm which has already been
 * initialized.
 * @param filename: The file to read.
 * @return Whether the image could be decoded.
 **************************************************************/
extern bool ppm_Decode(IMAGE *ppm, const char *filename);

/*============================================================*/
#endif // _PPM_H_