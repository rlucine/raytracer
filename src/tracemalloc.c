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
    fprintf(stderr, "malloc\t0x%p (size %zd, function %s)\n", ptr, size, where);
    return ptr;
}

void trace_free(void *ptr, const char *where) {
    fprintf(stderr, "free\t0x%p (function %s)\n", ptr, where);
    free(ptr);
    fprintf(stderr, "free succeeded\n");
}

void *trace_calloc(size_t nmemb, size_t size, const char *where) {
    void *ptr = calloc(nmemb, size);
    fprintf(stderr, "calloc\t0x%p (nmemb %zd, size %zd, function %s)\n", ptr, nmemb, size, where);
    return ptr;
}
    
void *trace_realloc(void *ptr, size_t size, const char *where) {
    void *next = realloc(ptr, size);
    fprintf(stderr, "realloc\t0x%p to 0x%p (size %zd, function %s)\n", ptr, next, size, where);
    return next;
}

/*============================================================*/
