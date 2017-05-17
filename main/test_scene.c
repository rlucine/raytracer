/**********************************************************//**
 * @file test_scene.c
 * @brief Test suite for all SCENE functions
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <float.h>      // DBL_EPSILON
#include <math.h>       // fabs

// This project
#include "scene.h"
#include "vector.h"
#include "image.h"
#include "shape.h"
#include "debug.h"

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    SCENE scene;
    VECTOR vector;
    
    // Decode a correct scene
    printf("Decoding test.scene...\n");
    assert(scene_Decode(&scene, "data/test.scene"));
    
    vector.x = vector.y = 0.0;
    vector.z = 4.0;
    assert(vector_IsEqual(scene_GetEyePosition(&scene), &vector));
    
    vector.x = vector.y = 0.0;
    vector.z = -1.0;
    assert(vector_IsEqual(scene_GetViewDirection(&scene), &vector));
    
    vector.x = vector.z = 0.0;
    vector.y = 1.0;
    assert(vector_IsEqual(scene_GetUpDirection(&scene), &vector));
    
    assert(fabs(scene_GetFieldOfView(&scene) - 60.0) < DBL_EPSILON);
    
    assert(scene_GetWidth(&scene) == 100);
    assert(scene_GetHeight(&scene) == 100);
    
    const COLOR *color = scene_GetBackgroundColor(&scene);
    assert(iszero(color->x - 0.1));
    assert(iszero(color->y - 0.1));
    assert(iszero(color->z - 0.1));
    
    assert(scene_GetNumberOfShapes(&scene) == 1);
    
    const SHAPE *shape = scene_GetShape(&scene, 0);
    assert(shape->shape == SHAPE_SPHERE);
    
    const SPHERE *sphere = shape_GetSphere(shape);
    assert(fabs(sphere->radius - 1.0) < DBL_EPSILON);
    vector.x = vector.y = vector.z = 0.0;
    assert(vector_IsEqual(&sphere->center, &vector));
    
    scene_Destroy(&scene);
    printf("Success!\n\n");

    // Decode a weird scene
    printf("Decoding weird.scene...\n");
    assert(scene_Decode(&scene, "data/weird.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");

    printf("Decoding empty.scene...\n");
    assert(scene_Decode(&scene, "data/empty.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding many.scene...\n");
    assert(scene_Decode(&scene, "data/many.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding sample0.scene...\n");
    assert(scene_Decode(&scene, "data/sample0.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding sample1.scene...\n");
    assert(scene_Decode(&scene, "data/sample1.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding sample2.scene...\n");
    assert(scene_Decode(&scene, "data/sample2.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding art.scene...\n");
    assert(scene_Decode(&scene, "data/art.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding test...\n");
    assert(scene_Decode(&scene, "data/test"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    printf("Decoding complex.scene...\n");
    assert(scene_Decode(&scene, "data/complex.scene"));
    scene_Destroy(&scene);
    printf("Success!\n\n");
    
    
    // Fail to decode these scenes
    printf("Catching errors in bad.scene...\n");
    assert(!scene_Decode(&scene, "data/bad.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in bounds.scene...\n");
    assert(!scene_Decode(&scene, "data/bounds.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in extra.scene...\n");
    assert(!scene_Decode(&scene, "data/extra.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in missing.scene...\n");
    assert(!scene_Decode(&scene, "data/missing.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in paralell.scene...\n");
    assert(!scene_Decode(&scene, "data/paralell.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in nonexistant file...\n");
    assert(!scene_Decode(&scene, "data/doesnotexist.scene"));
    printf("Success!\n\n");
    
    printf("Catching errors in bad file...\n");
    assert(!scene_Decode(&scene, "data"));
    printf("Success!\n\n");
    
    
    // Done!
    printf("Passed all tests!\n");
    return 0;
}
/*============================================================*/