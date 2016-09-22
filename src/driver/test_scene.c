/**********************************************************//**
 * @file test_scene.c
 * @brief Test suite for all SCENE functions
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON
#include <math.h>       // fabs

// This project
#include "scene.h"
#include "parser.h"
#include "vector.h"
#include "image.h"
#include "shape.h"

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    SCENE scene;
    VECTOR vector;
    
    // Decode a correct scene
    printf("Decoding test.scene...\n");
    assert(scene_Decode(&scene, "data/test.scene") == SUCCESS);
    
    assert(vector_IsZero(scene_GetEyePosition(&scene)));
    
    vector.x = vector.y = 0.0;
    vector.z = -1.0;
    assert(vector_IsEqual(scene_GetViewDirection(&scene), &vector));
    
    vector.x = vector.z = 0.0;
    vector.y = 1.0;
    assert(vector_IsEqual(scene_GetUpDirection(&scene), &vector));
    
    assert(fabs(scene_GetFieldOfView(&scene) - 60.0) < DBL_EPSILON);
    
    assert(scene_GetWidth(&scene) == 64);
    assert(scene_GetHeight(&scene) == 48);
    
    const RGB *color = scene_GetBackgroundColor(&scene);
    assert(color->r == 255);
    assert(color->g == 0);
    assert(color->b == 255);
    
    assert(scene_GetNumberOfShapes(&scene) == 1);
    
    const SHAPE *shape = scene_GetShape(&scene, 0);
    assert(shape_GetGeometry(shape) == SHAPE_SPHERE);
    
    const SPHERE *sphere = shape_GetSphere(shape);
    assert(fabs(sphere_GetRadius(sphere) - 3.8) < DBL_EPSILON);
    vector.x = vector.y = 0.0;
    vector.z = -4.0;
    assert(vector_IsEqual(sphere_GetCenter(sphere), &vector));
    
    scene_Destroy(&scene);
    printf("Success!\n\n");

    // Decode a weird scene
    printf("Decoding weird.scene...\n");
    assert(scene_Decode(&scene, "data/weird.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");

    printf("Decoding empty.scene...\n");
    assert(scene_Decode(&scene, "data/empty.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding many.scene...\n");
    assert(scene_Decode(&scene, "data/many.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding simple.scene...\n");
    assert(scene_Decode(&scene, "data/simple.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding tatest.scene...\n");
    assert(scene_Decode(&scene, "data/tatest.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding test...\n");
    assert(scene_Decode(&scene, "data/test") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding complex.scene...\n");
    assert(scene_Decode(&scene, "data/complex.scene") == SUCCESS);
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    
    // Fail to decode these scenes
    printf("Catching errors in bad.scene...\n");
    assert(scene_Decode(&scene, "data/bad.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in bounds.scene...\n");
    assert(scene_Decode(&scene, "data/bounds.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in extra.scene...\n");
    assert(scene_Decode(&scene, "data/extra.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in missing.scene...\n");
    assert(scene_Decode(&scene, "data/missing.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in paralell.scene...\n");
    assert(scene_Decode(&scene, "data/paralell.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in nonexistant file...\n");
    assert(scene_Decode(&scene, "data/doesnotexist.scene") == FAILURE);
    printf("Success!\n\n");
    
    printf("Catching errors in bad file...\n");
    assert(scene_Decode(&scene, "data") == FAILURE);
    printf("Success!\n\n");
    
    
    // Done!
    printf("Passed all tests!\n");
    return 0;
}
/*============================================================*/