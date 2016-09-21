/**********************************************************//**
 * @file tracemalloc.h
 * @brief Debugging memory errors. The TRACE macro must be
 * enabled on the command-line to enable this module in files
 * that include it.
 * This module defines macros to shadow malloc, calloc, realloc
 * and free and prints data to stderr about what blocks are
 * being allocated.
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _TRACEMALLOC_H_
#define _TRACEMALLOC_H_

// Standard library
#include <stdlib.h>     // size_t

/**********************************************************//**
 * @brief Trace a call to malloc
 * @param size: Size of block being allocated
 * @param where: The function invoking this call
 * @return Result of malloc(size)
 **************************************************************/
extern void *trace_malloc(size_t size, const char *where);

/**********************************************************//**
 * @brief Trace a call to free
 * @param ptr: The block of memory to free
 * @param where: The function invoking this call
 **************************************************************/
extern void trace_free(void *ptr, const char *where);

/**********************************************************//**
 * @brief Trace a call to calloc
 * @param nmemb: Number of members to allocate
 * @param size: Size of each member
 * @param where: The function invoking this call
 * @return Result of calloc(nmemb, size)
 **************************************************************/
extern void *trace_calloc(size_t nmemb, size_t size, const char *where);

/**********************************************************//**
 * @brief Trace a call to realloc
 * @param ptr: The block to reallocate
 * @param size: Size of block being allocated
 * @param where: The function invoking this call
 * @return Result of realloc(size)
 **************************************************************/
extern void *trace_realloc(void *ptr, size_t size, const char *where);

/*============================================================*
 * Overrides
 *============================================================*/
#ifdef TRACE

/**********************************************************//**
 * @def malloc
 * @brief Macro to shadow malloc if TRACE is defined
 **************************************************************/
#define malloc(size) trace_malloc(size, __FUNCTION__)

/**********************************************************//**
 * @def free
 * @brief Macro to shadow free if TRACE is defined
 **************************************************************/
#define free(ptr) trace_free(ptr, __FUNCTION__)

/**********************************************************//**
 * @def calloc
 * @brief Macro to shadow calloc if TRACE is defined
 **************************************************************/
#define calloc(nmemb, size) trace_calloc(nmemb, size, __FUNCTION__)

/**********************************************************//**
 * @def realloc
 * @brief Macro to shadow realloc if TRACE is defined
 **************************************************************/
#define realloc(ptr, size) trace_realloc(ptr, size, __FUNCTION__)
#endif

/*============================================================*/
#endif // _TRACEMALLOC_H_
