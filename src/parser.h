/**********************************************************//**
 * @file parser.h
 * @brief Header file for parsing scene files
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _PARSER_H_
#define _PARSER_H_

// This project
#include "macro.h"
#include "scene.h"

/**********************************************************//**
 * @brief Load a scene from a file. If this function succeeds
 * you must destroy the SCENE with scene_Destroy. If this
 * function fails all memory usage is cleaned up and you will
 * not need to use scene_Destroy.
 * @param scene: An uninitialized SCENE to read data into
 * @param filename: The file to read data from
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int scene_Decode(SCENE *scene, const char *filename);

/*============================================================*/
#endif // _PARSER_H_
