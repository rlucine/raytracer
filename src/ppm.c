/**********************************************************//**
 * @file ppm.c
 * @brief Implementation file for P3 PPM file parsing
 * @author Rena Shinomiya
 **************************************************************/

// Standard library
#include <stdbool.h>    // bool
#include <stdlib.h>     // malloc, free, size_t
#include <stdio.h>      // fopen, fclose, fprintf ...
#include <limits.h>     // USHRT_MAX
#include <string.h>     // strcpy

// This project
#include "image.h"      // IMAGE
#include "ppm.h"        // PPM_MAX_COLOR ...

// Debugging modules
#include "debug.h"      // eprintf, assert

/*============================================================*
 * Encoding PPM
 *============================================================*/
bool ppm_Encode(const IMAGE *ppm, const char *filename) {
    // Open the output file
    FILE *file = fopen(filename, "w");
    if (!file) {
        eprintf("Cannot open file %s\n", filename);
        return false;
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
    return true;
}

/**********************************************************//**
 * @brief Parse the next integer from the PPM stream.
 * @param file: An open PPM file stream.
 * @param output: The integer to output.
 * @return Whether an integer was parsed.
 **************************************************************/
bool ppm_Parse(FILE *file, int *output) {
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
            // Due to arbitrary definitions of true or false we need this
            return !in_comment && ungetc(current, file) != EOF && fscanf(file, "%d", output) == 1;
        }
    }
    return false;
}

/*============================================================*
 * Decoding PPM
 *============================================================*/
bool ppm_Decode(IMAGE *ppm, const char *filename) {
    // File for decoding
    FILE *file = fopen(filename, "r");
    if (!file) {
        eprintf("Failed to open file\n");
        return false;
    }
    
    // Read the header (always the first two bytes)
    char header[3];
    if (fscanf(file, "%2c", header) != 1) {
        eprintf("Failed to read header information\n");
        fclose(file);
        return false;
    }
    header[2] = '\0';
    if (header[0] != 'P' || header[1] != '3') {
        eprintf("Corrupt header '%s'\n", header);
        fclose(file);
        return false;
    }
    
    // Read header information
    int width;
    int height;
    int maxsize;
    bool failure = false;
    failure = failure || !ppm_Parse(file, &width);
    failure = failure || !ppm_Parse(file, &height);
    failure = failure || !ppm_Parse(file, &maxsize);
    if (failure) {
        eprintf("Failed to parse header information\n");
        fclose(file);
        return false;
    }
    
    // Read color information
    RGB rgb;
    if (!image_Create(ppm, width, height)) {
        eprintf("Failed to parse header information\n");
        fclose(file);
        return false;
    }
    
    // Read all information
    int x, y, red, blue, green;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            failure =false;
            failure = failure || !ppm_Parse(file, &red);
            failure = failure || !ppm_Parse(file, &green);
            failure = failure || !ppm_Parse(file, &blue);
            if (failure) {
                eprintf("Parse error\n");
                fclose(file);
                return false;
            }
            
            // Load color
            rgb.r = red * PPM_MAX_COLOR / maxsize;
            rgb.g = green * PPM_MAX_COLOR / maxsize;
            rgb.b = blue * PPM_MAX_COLOR / maxsize;
            if (!image_SetPixel(ppm, x, y, &rgb)) {
                eprintf("Failed to place colors in image\n");
                fclose(file);
                return false;
            }
        }
    }
    fclose(file);
    return true;
}

/*============================================================*/
