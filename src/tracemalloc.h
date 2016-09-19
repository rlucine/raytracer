/**********************************************************//**
 * @file tracemalloc.h
 * @brief Debugging memory errors
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _TRACEMALLOC_H_
#define _TRACEMALLOC_H_

// Standard library
#include <stdlib.h>     // size_t

/*============================================================*
 * Functions
 *============================================================*/
extern void *trace_malloc(size_t size, const char *where);
extern void trace_free(void *ptr, const char *where);
extern void *trace_calloc(size_t nmemb, size_t size, const char *where);
extern void *trace_realloc(void *ptr, size_t size, const char *where);

/*============================================================*
 * Overrides
 *============================================================*/
#define malloc(size) trace_malloc(size, __FUNCTION__)
#define free(ptr) trace_free(ptr, __FUNCTION__)
#define calloc(nmemb, size) trace_calloc(nmemb, size, __FUNCTION__)
#define realloc(ptr, size) trace_realloc(ptr, size, __FUNCTION__)

/*============================================================*/
#endif // _TRACEMALLOC_H_
