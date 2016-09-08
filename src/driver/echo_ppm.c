/**********************************************************//**
 * @file echo_ppm.c
 * @brief Testing program to load a PPm and then save it again
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <assert.h>     // assert
#include <string.h>     // strcmp

// This project
#include "ppm.h"

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(int argc, char **argv) {
    
    // Command-line args
    const char *filename = NULL;
    const char *output = NULL;
    if (argc > 2) {
        filename = argv[1];
        output = argv[2];
    }
    
    // Check for invalid input or help token
    if (argc <= 2 || !strcmp(filename, "-h") || !strcmp(filename, "--help")) {
        printf("Usage: %s filename output\n", argv[0]);
        printf(" filename: The name of the PPM to echo.\n");
        printf(" output: The destination to echo the PPM to.\n");
        return -1;
    }
    
    // Load PPM
    PPM decode;
    if (ppm_Decode(&decode, filename) != SUCCESS) {
        printf("Failed to decode %s\n", filename);
        return -1;
    }
    
    // Save PPM
    if (ppm_Encode(&decode, output) != SUCCESS) {
        printf("Failed to encode %s\n", output);
        return -1;
    }
    
    // Done
    ppm_Destroy(&decode);
    return 0;
}
/*============================================================*/