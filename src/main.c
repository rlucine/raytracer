/**********************************************************//**
 * @file main.c
 * @brief Main module for PPM testing
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // printf, fopen, fclose ...
#include <string.h>     // strcmp, strncmp, strlen ...

#ifdef DEBUG
#include <assert.h>     // assert
#endif

// This project
#include "ppm.h"

/**********************************************************//**
 * @brief Driver function for PPM tests
 **************************************************************/
int main(int argc, char **argv) {
    
    // Command-line args
    const char *filename = NULL;
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Check for invalid input or help token
    if (argc <= 1 || !strcmp(filename, "-h") || !strcmp(filename, "--help")) {
        printf("Usage: %s filename\n", argv[0]);
        printf(" filename: The name of a file containing the image specification.\n");
        printf(" The image is denoted by \"imsize width height\" where width and height\n");
        printf(" are positive integer values.\n");
        printf("The program will generate a .ppm file with the same base name.\n");
        return -1;
    }
    
    // Open the file
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open the file \"%s\"\n", filename);
        return -1;
    }
    
    // Read the file data
    const int BUF_SIZE = 255;
    char buf[BUF_SIZE + 1];
    if (!fgets(buf, BUF_SIZE, file)) {
        printf("Failed to read from the file \"%s\"\n", filename);
        fclose(file);
        return -1;
    }
    fclose(file);
    
    // Search for the correct format and harvest input variables
    int width;
    int height;
    const char *IMSIZE = "imsize";
    if (!strncmp(buf, IMSIZE, strlen(IMSIZE))) {
        // Found the correct token
        if (sscanf(buf + strlen(IMSIZE), "%d %d", &width, &height) != 2) {
            printf("Incorrect input file format\n");
            return -1;
        }
    }
    
    // Generate the PPM image
    PPM ppm;
    if (ppm_Create(&ppm, width, height) == FAILURE) {
        printf("Failed to create the ppm image\n");
        return -1;
    }
    
    // Fill the image with red
    RGB color;
    int x;
    int y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            color.r = (x * 8) % 256;
            color.g = (y * 8) % 256;
            color.b = (x + y) % 256;
            if (ppm_SetPixel(&ppm, x, y, &color) == FAILURE) {
                printf("Failed to set every pixel to red\n");
                return -1;
            }
#ifdef DEBUG
            const RGB *test;
            test = ppm_GetPixel(&ppm, x, y);
            assert(test->r == color.r);
            assert(test->g == color.g);
            assert(test->b == color.b);
#endif
        }
    }
    
    // Determine the output filename
    // buf padded with null bytes if smaller
    const char *PPM_EXT = "ppm\0";
    strncpy(buf, filename, BUF_SIZE);
    int k = BUF_SIZE - strlen(PPM_EXT);
    while (k >= 0 && buf[k] != '.') {
        k--;
    }
    if (k < 0) {
        printf("Failed to generate output filename\n");
        return -1;
    } else {
        strcpy(&buf[k+1], PPM_EXT);
    }
    
    // Encode the image
    if (ppm_Encode(&ppm, buf) == FAILURE) {
        printf("Failed to encode image at \"%s\"\n", buf);
        return -1;
    }
    
    // Clean up
    ppm_Destroy(&ppm);
    return 0;
}

/*============================================================*/