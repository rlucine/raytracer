/**********************************************************//**
 * @file ppm.c
 * @brief Implementation file for P3 PPM file parsing
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, size_t
#include <stdio.h>      // fopen, fclose, fprintf, getline ...
#include <limits.h>     // USHRT_MAX
#include <string.h>     // strcpy
#include <assert.h>     // assert

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "ppm.h"        // PPM_MAX_COLOR ...

// Debugging modules
#ifdef TRACE
#include "tracemalloc.h"
#endif

/*============================================================*
 * Encoding PPM
 *============================================================*/
int ppm_Encode(const IMAGE *ppm, const char *filename) {
    
    // Open the output file
    FILE *file = fopen(filename, "w");
    if (!file) {
#ifdef VERBOSE
        perror("fopen");
        fprintf(stderr, "ppm_Encode failed: Cannot open file %s\n", filename);
#endif
        return FAILURE;
    }
    
    // Encode the PPM header
    fprintf(file, "P3\n%d %d\n%d\n", ppm->width, ppm->height, PPM_MAX_COLOR);
    
    // Encode the color data
    int index = 0;
    int max = ppm->width * ppm->height;
    const RGB *color;
    while (index < max) {
        color = &ppm->data[index];
        fprintf(file, "%d %d %d\n", (int)color->r, (int)color->g, (int)color->b);
        index++;
    }
    
    // Done encoding
    fflush(file);
    fclose(file);
    return SUCCESS;
}

/*============================================================*
 * Decoding PPM
 *============================================================*/
int ppm_Parse(FILE *file, int *output) {
    // Get the next integer from the PPM stream
    char current;
    int in_comment = 0;
    
    while ((current = fgetc(file)) != EOF) {
        switch (current) {
        // Whitespace characters
        case '\n':
            in_comment = 0;
        case ' ':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
            continue;
        
        // Comment characters
        case '#':
            in_comment = 1;
            break;
        
        // Numeric characters
        default:
            // Due to arbitrary definitions of SUCCESS or FAILURE we need this
            if (!in_comment && !fseek(file, -1, SEEK_CUR) && fscanf(file, "%d", output) == 1) {
                return SUCCESS;
            } else {
                // Fails because found unparseable things
                return FAILURE;
            }
            break;
        }
    }
    return FAILURE;
}

int ppm_Decode(IMAGE *ppm, const char *filename) {
    
    // File for decoding
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Failed to open file\n");
#endif
        return FAILURE;
    }
    
    // Read the header (always the first two bytes)
    char header[2];
    if (fscanf(file, "%2c", header) != 1) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Failed to read header information\n");
#endif
        fclose(file);
        return FAILURE;
    }
    if (!strcmp(header, "P3")) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: File is not a P3 PPM image\n");
#endif
        fclose(file);
        return FAILURE;
    }
    
    // Read header information
    int width;
    int height;
    int maxsize;
    int failure = 0;
    failure = failure || (ppm_Parse(file, &width) != SUCCESS);
    failure = failure || (ppm_Parse(file, &height) != SUCCESS);
    failure = failure || (ppm_Parse(file, &maxsize) != SUCCESS);
    if (failure) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Failed to parse header information\n");
#endif
        fclose(file);
        return FAILURE;
    }
    
#ifdef DEBUG
    fprintf(stderr, "ppm_Decode: image is %d by %d\n", width, height);
    fprintf(stderr, "ppm_Decode: maxsize is %d\n", maxsize);
#endif
    
    // Read color information
    RGB rgb;
    if (image_Create(ppm, width, height) != SUCCESS) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Failed to parse header information\n");
#endif
        fclose(file);
        return FAILURE;
    }
    
    // Read all information
    int x, y, red, blue, green;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            failure = 0;
            failure = failure || (ppm_Parse(file, &red) != SUCCESS);
            failure = failure || (ppm_Parse(file, &green) != SUCCESS);
            failure = failure || (ppm_Parse(file, &blue) != SUCCESS);
            if (failure) {
#ifdef VERBOSE
                fprintf(stderr, "ppm_Decode failed: Parse error\n");
#endif
                fclose(file);
                return FAILURE;
            }
            
            // Load color
            rgb.r = red * PPM_MAX_COLOR / maxsize;
            rgb.g = green * PPM_MAX_COLOR / maxsize;
            rgb.b = blue * PPM_MAX_COLOR / maxsize;
            if (image_SetPixel(ppm, x, y, &rgb) != SUCCESS) {
#ifdef VERBOSE
                fprintf(stderr, "ppm_Decode failed: Failed to place colors in image\n");
#endif
                fclose(file);
                return FAILURE;
            }
        }
    }
    fclose(file);
    
    // Done, read all color information
    return SUCCESS;
}

/*============================================================*/
