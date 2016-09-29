/**********************************************************//**
 * @file light.h
 * @brief Header file for light definitions
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _LIGHT_H_
#define _LIGHT_H_

// This project
#include "macro.h"
#include "color.h"
#include "vector.h"
#include "shape.h"

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
 * @brief Calculate the Blinn-Phong shading for one light at the
 * given collision location.
 * @param light: The light to calculate shading for
 * @param collision: The point of collision to check.
 * @param view: The view vector of the scene
 * @param color: Output location for the color
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int light_BlinnPhongShade(const LIGHT *light, const COLLISION *collision, const VECTOR *view, COLOR *color);

/*============================================================*/
#endif // _LIGHT_H_
