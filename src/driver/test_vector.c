/**********************************************************//**
 * @file test_vector.c
 * @brief Test suite for all VECTOR functions
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // fprintf ...
#include <assert.h>     // assert
#include <float.h>      // DBL_EPSILON
#include <math.h>       // fabs, M_PI

// This project
#include "vector.h"

/*============================================================*
 * Double precision error assert
 *============================================================*/
#define assert_DoubleEqual(a, b) assert(fabs(a - b) < DBL_EPSILON)

/**********************************************************//**
 * @brief Test suite driver function
 **************************************************************/
int main(void) {
    VECTOR zero, i, j, k, copy;
    
    // Zero vector
    zero.x = zero.y = zero.z = 0.0;
    
    // X basis
    i.x = 1.0;
    i.y = i.z = 0.0;
    
    // Y basis
    j.y = 1.0;
    j.x = j.z = 0.0;
    
    // Z basis
    k.z = 1.0;
    k.x = k.y = 0.0;
    
    // Test zero-checking
    assert(vector_IsZero(&zero));
    assert(!vector_IsZero(&i));
    assert(!vector_IsZero(&j));
    assert(!vector_IsZero(&k));
    
    // Test copying
    vector_Copy(&copy, &zero);
    assert(vector_IsEqual(&copy, &zero));
    
    // Test dot product
    assert_DoubleEqual(vector_Dot(&i, &i), 1.0);
    assert_DoubleEqual(vector_Dot(&j, &j), 1.0);
    assert_DoubleEqual(vector_Dot(&k, &k), 1.0);
    assert_DoubleEqual(vector_Dot(&i, &j), 0.0);
    assert_DoubleEqual(vector_Dot(&j, &k), 0.0);
    assert_DoubleEqual(vector_Dot(&i, &k), 0.0);
    assert_DoubleEqual(vector_Dot(&zero, &i), 0.0);
    
    // Test magnitude
    assert_DoubleEqual(vector_Magnitude(&i), 1.0);
    assert_DoubleEqual(vector_Magnitude(&j), 1.0);
    assert_DoubleEqual(vector_Magnitude(&k), 1.0);
    assert_DoubleEqual(vector_Magnitude(&zero), 0.0);
    
    // Test angle
    assert_DoubleEqual(vector_Angle(&i, &j), M_PI / 2);
    assert_DoubleEqual(vector_Angle(&j, &k), M_PI / 2);
    assert_DoubleEqual(vector_Angle(&i, &k), M_PI / 2);
    assert_DoubleEqual(vector_Angle(&i, &i), 0.0);
    
    // Test paralell
    assert(vector_IsParalell(&i, &i));
    assert(vector_IsParalell(&j, &j));
    assert(vector_IsParalell(&k, &k));
    assert(!vector_IsParalell(&i, &j));
    assert(!vector_IsParalell(&j, &k));
    assert(!vector_IsParalell(&i, &k));
    
    // Test orthogonal
    assert(vector_IsOrthogonal(&i, &j));
    assert(vector_IsOrthogonal(&j, &k));
    assert(vector_IsOrthogonal(&i, &k));
    assert(!vector_IsOrthogonal(&i, &i));
    
    // Test equal
    assert(vector_IsEqual(&i, &i));
    assert(vector_IsEqual(&j, &j));
    assert(vector_IsEqual(&k, &k));
    assert(!vector_IsEqual(&i, &j));
    assert(!vector_IsEqual(&j, &k));
    assert(!vector_IsEqual(&i, &k));
    
    // Test unit
    assert(vector_IsUnit(&i));
    assert(vector_IsUnit(&j));
    assert(vector_IsUnit(&k));
    assert(!vector_IsUnit(&zero));
    
    // Test add
    vector_Copy(&copy, &zero);
    vector_Add(&copy, &i, &zero);
    assert(vector_IsEqual(&copy, &i));
    vector_Add(&copy, &j, &zero);
    assert(vector_IsEqual(&copy, &j));
    vector_Add(&copy, &k, &zero);
    assert(vector_IsEqual(&copy, &k));
    
    // Test subtract
    vector_Subtract(&copy, &i, &i);
    assert(vector_IsZero(&copy));
    vector_Subtract(&copy, &j, &j);
    assert(vector_IsZero(&copy));
    vector_Subtract(&copy, &k, &k);
    assert(vector_IsZero(&copy));
    
    vector_Subtract(&copy, &i, &zero);
    assert(vector_IsEqual(&copy, &i));
    vector_Subtract(&copy, &j, &zero);
    assert(vector_IsEqual(&copy, &j));
    vector_Subtract(&copy, &k, &zero);
    assert(vector_IsEqual(&copy, &k));
    
    // Test negate
    vector_Negate(&copy, &i);
    assert(!vector_IsEqual(&copy, &i));
    vector_Negate(&copy, &copy);
    assert(vector_IsEqual(&copy, &i));
    
    vector_Negate(&copy, &j);
    assert(!vector_IsEqual(&copy, &j));
    vector_Negate(&copy, &copy);
    assert(vector_IsEqual(&copy, &j));
    
    vector_Negate(&copy, &k);
    assert(!vector_IsEqual(&copy, &k));
    vector_Negate(&copy, &copy);
    assert(vector_IsEqual(&copy, &k));
    
    // Test cross
    vector_Cross(&copy, &i, &j);
    assert(vector_IsEqual(&copy, &k));
    vector_Cross(&copy, &i, &copy);
    assert(vector_IsEqual(&copy, &j));
    vector_Cross(&copy, &copy, &k);
    assert(vector_IsEqual(&copy, &i));
    
    // Test unit
    vector_Copy(&copy, &i);
    vector_Multiply(&copy, &copy, 2);
    assert(!vector_IsEqual(&copy, &i));
    vector_Unit(&copy, &copy);
    assert(vector_IsUnit(&copy));
    
    vector_Copy(&copy, &j);
    vector_Multiply(&copy, &copy, 2);
    assert(!vector_IsEqual(&copy, &j));
    vector_Unit(&copy, &copy);
    assert(vector_IsUnit(&copy));
    
    vector_Copy(&copy, &k);
    vector_Multiply(&copy, &copy, 2);
    assert(!vector_IsEqual(&copy, &k));
    vector_Unit(&copy, &copy);
    assert(vector_IsUnit(&copy));
    
    // Test multiply
    vector_Multiply(&copy, &zero, 4);
    assert(vector_IsEqual(&copy, &zero));
    
    vector_Multiply(&copy, &i, 4);
    vector_Multiply(&copy, &copy, 0.25);
    assert(vector_IsEqual(&copy, &i));
    
    vector_Multiply(&copy, &j, 4);
    vector_Multiply(&copy, &copy, 0.25);
    assert(vector_IsEqual(&copy, &j));
    
    vector_Multiply(&copy, &k, 4);
    vector_Multiply(&copy, &copy, 0.25);
    assert(vector_IsEqual(&copy, &k));
    
    // Done
    printf("Passed all tests!\n");
    return 0;
}
/*============================================================*/