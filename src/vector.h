/**********************************************************//**
 * @file vector.h
 * @brief Header file for VECTOR operations
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _VECTOR_H_
#define _VECTOR_H_

/**********************************************************//**
 * @typedef VECTOR
 * @brief Struct for 3-dimensional vectors
 **************************************************************/
typedef struct {
    double x;   ///< The x coordinate of the vector
    double y;   ///< The y coordinate of the vector
    double z;   ///< The z coordinate of the vector
} VECTOR;

/**********************************************************//**
 * @typedef POINT
 * @brief Struct for 3-dimensional points
 **************************************************************/
typedef VECTOR POINT;

// TODO maybe this belongs in another file?
/**********************************************************//**
 * @typedef LINE
 * @brief Struct for an infinite line
 **************************************************************/
typedef struct {
    POINT origin;       ///< Origin of the line
    VECTOR direction;   ///< Direction of the line
} LINE;

// TODO maybe this belongs in another file?
/**********************************************************//**
 * @typedef PLANE
 * @brief Struct for a flat plane
 **************************************************************/
typedef struct {
    POINT origin;   ///< Upper left corner of the plane
    VECTOR basis_u; ///< Step of width in the plane
    VECTOR basis_v; ///< Step of height in the plane
} PLANE;

/**********************************************************//**
 * @brief Copy the input vector to the output vector
 * @param output: The vector to copy to.
 * @param input: The vector to copy.
 **************************************************************/
extern void vector_Copy(VECTOR *output, const VECTOR *input);

/**********************************************************//**
 * @brief Compute the dot product of the vectors
 * @param a: The first vector
 * @param b: The second vector
 * @return The value of a dotted with b
 **************************************************************/
extern double vector_Dot(const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Get the magnitude (length) of the vector
 * @param vector: The vector
 * @return The magnitude of the vector
 **************************************************************/
extern double vector_Magnitude(const VECTOR *vector);

/**********************************************************//**
 * @brief Get the angle between the two vectors in radians.
 * The angle is always from 0 to M_PI.
 * @param a: The first vector
 * @param b: The second vector
 * @return The angle in between the vectors
 **************************************************************/
extern double vector_Angle(const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Check if a vector is the zero vector
 * @param vector: The vector to check
 * @return 1 if it is the zero vector, otherwise 0
 **************************************************************/
extern int vector_IsZero(const VECTOR *vector);

/**********************************************************//**
 * @brief Check if two vectors are parelell
 * @param a: The first vector
 * @param b: The second vector
 * @return 1 if the vectors are parelell, otherwise 0
 **************************************************************/
extern int vector_IsParalell(const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Check if two vectors are orthogonal
 * @param a: The first vector
 * @param b: The second vector
 * @return 1 if the vectors are orthogonal, otherwise 0
 **************************************************************/
extern int vector_IsOrthogonal(const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Check if two vectors are equal
 * @param a: The first vector
 * @param b: The second vector
 * @return 1 if the vectors are equal, otherwise 0
 **************************************************************/
extern int vector_IsParalell(const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Check if a vector is of unit length
 * @param vector: The vector to check
 * @return 1 if it is a unit vector, otherwise 0
 **************************************************************/
extern int vector_IsUnit(const VECTOR *vector);

/**********************************************************//**
 * @brief Add the two input vectors
 * @param output: The result of the computation. This may be
 * equal to a or b without becoming incorrect.
 * @param a: The first vector
 * @param b: The second vector
 **************************************************************/
extern void vector_Add(VECTOR *output, const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Subtract the second vector from the first vector
 * @param output: The result of the computation. This may be
 * equal to a or b without becoming incorrect.
 * @param a: The first vector
 * @param b: The second vector
 **************************************************************/
extern void vector_Subtract(VECTOR *output, const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Negate the input vector
 * @param output: The result of the computation. This may be
 * equal to a without becoming incorrect.
 * @param a: The vector
 **************************************************************/
extern void vector_Negate(VECTOR *output, const VECTOR *a);

/**********************************************************//**
 * @brief Compute the cross product of a and b
 * @param output: The result of the computation. This may be
 * equal to a or b without becoming incorrect.
 * @param a: The first vector
 * @param b: The second vector
 **************************************************************/
extern void vector_Cross(VECTOR *output, const VECTOR *a, const VECTOR *b);

/**********************************************************//**
 * @brief Compute the unit vector in the direction of the
 * input vector. If the input vector is the zero vector, the
 * result will be the zero vector.
 * @param output: The result of the computation. This may be
 * equal to a without becoming incorrect.
 * @param a: The vector
 **************************************************************/
extern void vector_Unit(VECTOR *output, const VECTOR *a);

/**********************************************************//**
 * @brief Multiply the vector by a scalar
 * @param output: The result of the computation. This may be
 * equal to a without becoming incorrect.
 * @param a: The vector
 * @param scale: The value to multiply the vector with
 **************************************************************/
extern void vector_Multiply(VECTOR *output, const VECTOR *a, double scale);

/*============================================================*/
#endif // _VECTOR_H_
