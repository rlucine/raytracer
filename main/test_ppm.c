/**********************************************************//**
 * @file test_ppm.c
 * @brief Test suite for all PPM functions
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <assert.h>     // assert

// This project
#include "macro.h"
#include "image.h"
#include "ppm.h"

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    
    IMAGE ppm;
    int width = 32;
    int height = 64;
    
    // Generate the PPM image
    assert(image_Create(&ppm, width, height) == SUCCESS);
    assert(image_GetWidth(&ppm) == width);
    assert(image_GetHeight(&ppm) == height);
    image_Destroy(&ppm);
    
    // Generate another PPM
    width = 64;
    height = 32;
    assert(image_Create(&ppm, width, height) == SUCCESS);
    assert(image_GetWidth(&ppm) == width);
    assert(image_GetHeight(&ppm) == height);
    
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
            assert(image_SetPixel(&ppm, x, y, &color) == SUCCESS);
            test = image_GetPixel(&ppm, x, y);
            assert(test != NULL);
            assert(test->r == color.r);
            assert(test->g == color.g);
            assert(test->b == color.b);
        }
    }
    
    // Check error handling of invalid indexes
    assert(image_SetPixel(&ppm, width, height, &color) == FAILURE);
    assert(image_GetPixel(&ppm, width, height) == NULL);
    
    // Encode and decode test
    const char *filename = "test_ppm.ppm";
    assert(ppm_Encode(&ppm, filename) == SUCCESS);
    
    IMAGE decode;
    assert(ppm_Decode(&decode, filename) == SUCCESS);
    
    // Check image equality
    assert(image_GetWidth(&decode) == width);
    assert(image_GetHeight(&decode) == height);
    const RGB *first, *second;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            first = image_GetPixel(&ppm, x, y);
            second = image_GetPixel(&decode, x, y);
            assert(first->r == second->r);
            assert(first->g == second->g);
            assert(first->b == second->b);
        }
    }
    
    // Done
    image_Destroy(&ppm);
    image_Destroy(&decode);
    printf("Passed all tests!\n");
    return 0;
}
/*============================================================*/