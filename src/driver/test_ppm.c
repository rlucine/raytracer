/**********************************************************//**
 * @file test_ppm.c
 * @brief Test suite for ppm.c
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <assert.h>     // assert

// This project
#include "ppm.h"

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    
    PPM ppm;
    int width = 32;
    int height = 64;
    
    // Generate the PPM image
    assert(ppm_Create(&ppm, width, height) == SUCCESS);
    assert(ppm_GetWidth(&ppm) == width);
    assert(ppm_GetHeight(&ppm) == height);
    ppm_Destroy(&ppm);
    
    // Generate another PPM
    width = 64;
    height = 32;
    assert(ppm_Create(&ppm, width, height) == SUCCESS);
    assert(ppm_GetWidth(&ppm) == width);
    assert(ppm_GetHeight(&ppm) == height);
    
    // Fill the image with red
    RGB color;
    const RGB *test;
    int x;
    int y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            color.r = (x * 8) % 256;
            color.g = (y * 8) % 256;
            color.b = (x + y) % 256;
            assert(ppm_SetPixel(&ppm, x, y, &color) == SUCCESS);
            test = ppm_GetPixel(&ppm, x, y);
            assert(test != NULL);
            assert(test->r == color.r);
            assert(test->g == color.g);
            assert(test->b == color.b);
        }
    }
    
    // Check error handling of invalid indexes
    assert(ppm_SetPixel(&ppm, width, height, &color) == FAILURE);
    assert(ppm_GetPixel(&ppm, width, height) == NULL);
    
    // Encode and decode test
    const char *filename = "test_ppm.ppm";
    assert(ppm_Encode(&ppm, filename) == SUCCESS);
    
    PPM decode;
    assert(ppm_Decode(&decode, filename) == SUCCESS);
    
    // Check image equality
    assert(ppm_GetWidth(&decode) == width);
    assert(ppm_GetHeight(&decode) == height);
    const RGB *first, *second;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            first = ppm_GetPixel(&ppm, x, y);
            second = ppm_GetPixel(&decode, x, y);
            assert(first->r == second->r);
            assert(first->g == second->g);
            assert(first->b == second->b);
        }
    }
    
    // Done
    ppm_Destroy(&ppm);
    ppm_Destroy(&decode);
    printf("Passed all tests!\n");
    return 0;
}
/*============================================================*/