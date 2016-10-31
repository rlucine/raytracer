/**********************************************************//**
 * @file light.h
 * @brief Header file for light definitions
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _LIGHT_H_
#define _LIGHT_H_

// This project
#include "color.h"      // COLOR
#include "vector.h"     // VECTOR
#include "shape.h"      // COLLISION

/**********************************************************//**
 * @def MAX_ANGLE
 * @brief The maximum angle for a spotlight
 **************************************************************/
#define MAX_ANGLE 360

/**********************************************************//**
 * @def MIN_ANGLE
 * @brief The minimum angle for a spotlight
 **************************************************************/
#define MIN_ANGLE 0

/**********************************************************//**
 * @enum LIGHT_TYPE
 * @brief Keys representing every kind of LIGHT distribution
 **************************************************************/
typedef enum {
    LIGHT_NONE = 0, ///< Null value for uninitialized lights
    LIGHT_POINT,    ///< The light is a point source
    LIGHT_DIRECTED, ///< The light is purely directed
    LIGHT_SPOT,     ///< The light is a spotlight
} LIGHT_TYPE;

/**********************************************************//**
 * @struct LIGHT
 * @brief Struct containing all light information. LIGHT_POINT
 * need not define direction. LIGHT_DIRECTED need not define
 * where. Only LIGHT_SPOT requires angle.
 **************************************************************/
typedef struct {
    LIGHT_TYPE type;    ///< The type of light represented
    POINT where;        ///< The location of the light
    VECTOR direction;   ///< The direction the light points
    COLOR color;        ///< The color of the light
    double angle;       ///< LIGHT_SPOT angle of spread
} LIGHT;

/**********************************************************//**
 * @brief Set up a new point light.
 * @param light: The light to initialize.
 * @param where: The location of the light's center.
 * @param color: The color the light emits.
 **************************************************************/
extern void light_CreatePoint(LIGHT *light, const POINT *where, const COLOR *color);

/**********************************************************//**
 * @brief Set up a new directional light.
 * @param light: The light to initialize.
 * @param direction: The direction the light shines towards.
 * @param color: The color the light emits.
 **************************************************************/
extern void light_CreateDirected(LIGHT *light, const VECTOR *direction, const COLOR *color);

/**********************************************************//**
 * @brief Set up a new spotlight.
 * @param light: The light to initialize.
 * @param point: The location the light shines from.
 * @param direction: The direction the light shines towards.
 * @param angle: The light will hit objects within this angle
 * of the direction parameter.
 * @param color: The color the light emits.
 **************************************************************/
extern void light_CreateSpotlight(LIGHT *light, const POINT *point, const VECTOR *direction, double angle, const COLOR *color);

/**********************************************************//**
 * @brief Get the direction to the light.
 * @param light: The light to calculate shading for
 * @param where: The point to calculate the direction from.
 * @param output: The resulting direction as a unit vector.
 * @param distance: The distance to the light. This may be a
 * NULL pointer. Otherwise the distance is written to the
 * location.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int light_GetDirection(const LIGHT *light, const POINT *where, VECTOR *output, double *distance);

/**********************************************************//**
 * @brief Calculate the Blinn-Phong shading for one light at the
 * given collision location.
 * @param light: The light to calculate shading for
 * @param collision: The point of collision to check.
 * @param color: Output location for the color
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, COLOR *color);

/*============================================================*/
#endif // _LIGHT_H_
