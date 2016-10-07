/**********************************************************//**
 * @file ppm.c
 * @brief Implementation file for P3 PPM file parsing
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, size_t
#include <stdio.h>      // fopen, fclose, fprintf, getline ...
#include <limits.h>     // USHRT_MAX
#include <sys/stat.h>   // stat, struct stat
#include <string.h>     // strcpy
#include <assert.h>     // assert

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "image.h"      // IMAGE
#include "ppm.h"        // PPM_MAX_COLOR ...

// Debugging modules
#ifdef DEBUG
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
    fprintf(file, "P3 %d %d %d\n", ppm->width, ppm->height, PPM_MAX_COLOR);
    
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
int ppm_Decode(IMAGE *ppm, const char *filename) {
    
    // Decoding stuff
    const int BUF_SIZE = PPM_MAX_LINE;
    char buf[BUF_SIZE+1];

    // Calculate the size of the PPM image in bytes
    struct stat stat_buf;
    if (stat(filename, &stat_buf)) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: File \"%s\" not found\n", filename);
#endif
        return FAILURE;
    }
    
    // Allocate space for entire file in memory
    size_t size = stat_buf.st_size;
    char *entire = (char *)malloc(size+1);
    if (!entire) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Out of heap space\n");
#endif
        return FAILURE;
    }
    
    // Mark the end of the buffer
    const char CANARY_VALUE = 127;
    entire[size] = CANARY_VALUE;
#ifdef VERBOSE
    assert(entire[size] == CANARY_VALUE);
#endif
    
    // File for decoding
    FILE *file = fopen(filename, "r");
    if (!file) {
#ifdef VERBOSE
        perror("fopen");
        fprintf(stderr, "ppm_Decode failed: Failed to open file\n");
#endif
        free(entire);
        return FAILURE;
    }
    
    // Excise comments as we load into memory
    char *where = entire;
    size_t ncopy;
    while (fgets(buf, BUF_SIZE, file)) {
        if (buf[0] != '#') {
            // Not a comment line in the PPM so store it
            ncopy = strlen(buf);
            memcpy(where, buf, ncopy);
            where += ncopy;
        }
    }
    
    // Done with the file
    fclose(file);
    
    // Check buffer overflow
    if (entire[size] != CANARY_VALUE) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Buffer overflow detected (canary %d)\n", entire[size]);
#endif
        free(entire);
        return FAILURE;
    }
    
    // Now parse the PPM data which is free of comments
    int nread;
    char header[2];
    int width;
    int height;
    int maxsize;
    where = entire;
    if (sscanf(where, "%s %d %d %d%n", header, &width, &height, &maxsize, &nread) != 4) {
        // Failed to read entire header data
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Error parsing header\n");
#endif
        free(entire);
        return FAILURE;
    }
    
    // Header check
    if (header[0] != 'P' || header[1] != '3') {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Missing PPM magic number\n");
#endif
        free(entire);
        return FAILURE;
    }
    
    // Now have header information, can construct the PPM
    if (image_Create(ppm, width, height) == FAILURE) {
#ifdef VERBOSE
        fprintf(stderr, "ppm_Decode failed: Unable to create PPM\n");
#endif
        free(entire);
        return FAILURE;
    }
    
    // Read information from the PPM
    where += nread;
    RGB *color;
    int index = 0;
    int max = ppm->width * ppm->height;
    int first, second, third;
    while (index < max) {
        color = &ppm->data[index];
        if (sscanf(where, "%d %d %d%n", &first, &second, &third, &nread) != 3) {
            // Failed to read another color
#ifdef VERBOSE
            fprintf(stderr, "ppm_Decode failed: Error parsong color\n");
#endif
            free(entire);
            return FAILURE;
        }
        
        // Scale max size to 255
        if (maxsize != PPM_MAX_COLOR) {
            first = (first * PPM_MAX_COLOR) / maxsize;
            second = (second * PPM_MAX_COLOR) / maxsize;
            third = (third * PPM_MAX_COLOR) / maxsize;
        }
        
        // Set color
        color->r = (char)first;
        color->g = (char)second;
        color->b = (char)third;
        
        // Next color location
        where += nread;
        index += 1;
    }
    
    // Done, read all color information
    free(entire);
    return SUCCESS;
}

/*============================================================*/
