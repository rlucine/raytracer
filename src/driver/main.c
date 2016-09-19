/**********************************************************//**
 * @file main.c
 * @brief Main module for PPM testing
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc
#include <stdio.h>      // printf, fopen, fclose ...
#include <string.h>     // strcmp, strncmp, strlen ...
#include <assert.h>     // assert

// This project
#include "macro.h"
#include "ppm.h"
#include "scene.h"
#include "raytrace.h"

/**********************************************************//**
 * @brief Driver function for raytracer
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
        printf("\tfilename: The name of a file containing the image specification.\n");
        printf("The program will generate a .ppm file with the same base name.\n");
        return -1;
    }
    
    // Determine the output filename
    const char *EXTENSION = "ppm\0";
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
        index = end;
    }
    strcpy(&buf[index], EXTENSION);

    // Get the scene
    SCENE scene;
    if (scene_Decode(&scene, filename) != SUCCESS) {
        printf("Failed to decode the scene file \"%s\"\n", filename);
        return -1;
    }
    
    // Render the image
    PPM ppm;
    if (raytrace_Render(&ppm, &scene) != SUCCESS) {
        printf("Failed to render the image\n");
        return -1;
    }
    
    // Clean up scene
    scene_Destroy(&scene);
    
    // Encode the image
    if (ppm_Encode(&ppm, buf) != SUCCESS) {
        // Try to preserve the data
        printf("Failed to encode image at \"%s\"\n", buf);
        if (ppm_Encode(&ppm, "temp") != SUCCESS) {
            printf("Failed to buffer the image in a temp file\n");
        } else {
            printf("Successfully buffered image in a temporary file\n");
        }
        return -1;
    }
    
    // Clean up
    free(buf);
    ppm_Destroy(&ppm);
    printf("Success!\n");
    return 0;
}

/*============================================================*/