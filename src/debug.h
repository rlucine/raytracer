/**********************************************************//**
 * @file debug.h
 * @brief Debugging memory errors. The TRACE macro must be
 * enabled on the command-line to enable this module in files
 * that include it.
 * This module defines macros to shadow malloc, calloc, realloc
 * and free and prints data to stderr about what blocks are
 * being allocated.
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _DEBUG_H_
#define _DEBUG_H_

// Standard library
#include <stdlib.h>     // size_t
#include <stdio.h>      // fprintf, stderr

/*============================================================*
 * Malloc tracing
 *============================================================*/
#ifdef TRACE
#define malloc(size) debug_malloc(size, __FILE__, __LINE__, __func__)
#define free(ptr) debug_free(ptr, __FILE__, __LINE__, __func__)
#define calloc(nmemb, size) debug_calloc(nmemb, size, __FILE__, __LINE__, __func__)
#define realloc(ptr, size) debug_realloc(ptr, size, __FILE__, __LINE__, __func__)
#endif

/**********************************************************//**
 * @brief Trace a call to malloc
 * @param size: Size of block being allocated
 * @param file: The file invoking this call.
 * @param line: The line number of the file.
 * @param function: The function invoking this call
 * @return Result of malloc(size)
 **************************************************************/
extern void *debug_malloc(size_t size, const char *file, int line, const char *function);

/**********************************************************//**
 * @brief Trace a call to free
 * @param ptr: The block of memory to free
 * @param where: The function invoking this call
 **************************************************************/
extern void debug_free(void *ptr, const char *file, int line, const char *function);

/**********************************************************//**
 * @brief Trace a call to calloc
 * @param nmemb: Number of members to allocate
 * @param size: Size of each member
 * @param where: The function invoking this call
 * @return Result of calloc(nmemb, size)
 **************************************************************/
extern void *debug_calloc(size_t nmemb, size_t size, const char *file, int line, const char *function);

/**********************************************************//**
 * @brief Trace a call to realloc
 * @param ptr: The block to reallocate
 * @param size: Size of block being allocated
 * @param where: The function invoking this call
 * @return Result of realloc(size)
 **************************************************************/
extern void *debug_realloc(void *ptr, size_t size, const char *file, int line, const char *function);

/*============================================================*
 * Debug printing
 *============================================================*/
#ifdef VERBOSE
#define errmsg(...) fprintf(stderr, "%s: ", __func__); fprintf(stderr, __VA_ARGS__)
#else
#define errmsg(...) (void)0
#endif

/*============================================================*/
#endif // _DEBUG_H_
