/**********************************************************//**
 * @file raytracer.c
 * @brief Main module for raytracer
 * @author Rena Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, srand
#include <time.h>       // time
#include <stdio.h>      // printf, fopen, fclose ...
#include <string.h>     // strcmp, strncmp, strlen ...

// This project
#include "image.h"
#include "ppm.h"
#include "scene.h"
#include "raytrace.h"

// Debugging libraries
#include "debug.h"      // eprintf, assert

/**********************************************************//**
 * @brief Driver function for raytracer
 **************************************************************/
int main(int argc, char **argv) {
    
    // Random number generator setup
#ifdef DEBUG
    srand(42);
#else
    srand(time(NULL));
#endif
    
    // Command-line args
    const char *filename = NULL;
    if (argc > 1) {
        filename = argv[1];
    }
    
    // Check for invalid input or help token
    if (argc <= 1 || !strcmp(filename, "-h") || !strcmp(filename, "--help")) {
        printf("Usage: %s filename\n", argv[0]);
        printf("\tfilename: The name of a file containing the image specification.\n");
        printf("The program will generate a .ppm file with the same base name.\n");
        return -1;
    }

    // Get the scene
    SCENE scene;
    if (!scene_Decode(&scene, filename)) {
        printf("Failed to decode the scene file \"%s\"\n", filename);
        return -1;
    }
    
    // Render the image
    IMAGE image;
    if (!raytrace_Render(&image, &scene)) {
        printf("Failed to render the image\n");
        scene_Destroy(&scene);
        return -1;
    }
    scene_Destroy(&scene);
    
    // Determine the output filename
    const char *EXTENSION = ".ppm\0";
    char *buf = (char *)malloc(sizeof(char) * (strlen(filename) + strlen(EXTENSION) + 1));
    if (!buf) {
        printf("Out of memory\n");
        return -1;
    }
    strcpy(buf, filename);
    
    // Look for the end or the . separator
    int end = strlen(buf) - 1;
    int index = end;
    while (buf[index] != '.' && buf[index] != '/' && buf[index] != '\\' && index >= 0) {
        index--;
    }
    if (index < 0 || buf[index] != '.') {
        index = end + 1;
    }
    strcpy(&buf[index], EXTENSION);
    
    // Encode the image
    if (!ppm_Encode(&image, buf)) {
        // Try to preserve the data
        printf("Failed to encode image at \"%s\"\n", buf);
        if (!ppm_Encode(&image, "temp")) {
            printf("Failed to buffer the image in a temp file\n");
        } else {
            printf("Successfully buffered image in a temporary file\n");
        }
        free(buf);
        return -1;
    }
    
    // Clean up
    free(buf);
    image_Destroy(&image);
    printf("Success!\n");
    return 0;
}

/*============================================================*/
