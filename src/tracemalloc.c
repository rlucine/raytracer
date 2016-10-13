/**********************************************************//**
 * @file tracemalloc.c
 * @brief Debugging memory errors
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, realloc
#include <stdio.h>      // stderr, fprintf

/*============================================================*
 * Functions
 *============================================================*/
void *trace_malloc(size_t size, const char *where) {
    void *ptr = malloc(size);
    fprintf(stderr, "malloc\t0x%p (size %d, function %s)\n", ptr, (int)size, where);
    return ptr;
}

void trace_free(void *ptr, const char *where) {
    fprintf(stderr, "free\t0x%p (function %s)\n", ptr, where);
    free(ptr);
}

void *trace_calloc(size_t nmemb, size_t size, const char *where) {
    void *ptr = calloc(nmemb, size);
    fprintf(stderr, "calloc\t0x%p (nmemb %d, size %d, function %s)\n", ptr, (int)nmemb, (int)size, where);
    return ptr;
}
    
void *trace_realloc(void *ptr, size_t size, const char *where) {
    void *next = realloc(ptr, size);
    fprintf(stderr, "realloc\t0x%p to 0x%p (size %d, function %s)\n", ptr, next, (int)size, where);
    return next;
}

/*============================================================*/
