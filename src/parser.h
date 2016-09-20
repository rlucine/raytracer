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
 * @brief Load a scene from a file
 * @param scene: An uninitialized SCENE to read data into
 * @param filename: The file to read
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int scene_Decode(SCENE *scene, const char *filename);

/*============================================================*/
#endif // _PARSER_H_
