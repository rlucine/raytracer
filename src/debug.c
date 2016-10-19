/**********************************************************//**
 * @file debug.h
 * @brief Debugging library implementation.
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // malloc, free, realloc
#include <stdio.h>      // stderr, fprintf

/*============================================================*
 * Malloc tracing
 *============================================================*/
void *debug_malloc(size_t size, const char *file, int line, const char *function) {
    void *ptr = malloc(size);
    fprintf(stderr, "malloc(%ld) returned 0x%p (at %s:%s:%d)\n", (long)size, ptr, file, function, line);
    return ptr;
}

void debug_free(void *ptr, const char *file, int line, const char *function) {
    fprintf(stderr, "free(0x%p) (at %s:%s:%d)\n", ptr, file, function, line);
    free(ptr);
}

void *debug_calloc(size_t nmemb, size_t size, const char *file, int line, const char *function) {
    void *ptr = calloc(nmemb, size);
    fprintf(stderr, "calloc(%ld, %ld) returned 0x%p (at %s:%s:%d)\n", (long)nmemb, (long)size, ptr, file, function, line);
    return ptr;
}
    
void *debug_realloc(void *ptr, size_t size, const char *file, int line, const char *function) {
    void *next = realloc(ptr, size);
    fprintf(stderr, "realloc(0x%p, %ld) returned 0x%p (at %s:%s:%d)\n", ptr, (long)size, next, file, function, line);
    return next;
}

/*============================================================*/
