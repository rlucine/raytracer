/**********************************************************//**
 * @file ppm.h
 * @brief Header file for P3 PPM file parsing
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _PPM_H_
#define _PPM_H_

// This project
#include "macro.h"
#include "image.h"

/**********************************************************//**
 * @brief Encode a PPM structure as a P3 PPM image with
 * the given filename.
 * @param ppm: The initialized PPM data to encode
 * @param filename: The target filename
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_Encode(const IMAGE *ppm, const char *filename);

/**********************************************************//**
 * @brief Load a PPM image from a file
 * @param ppm: Pointer to an uninitialized PPM struct. You need
 * to use ppm_Destroy on this parameter if the function is
 * successful. Don't pass a ppm which has already been
 * initialized.
 * @param filename: The file to read.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int ppm_Decode(IMAGE *ppm, const char *filename);

/*============================================================*/
#endif // _PPM_H_