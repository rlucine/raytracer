/**********************************************************//**
 * @file scanner.c
 * @brief Implementation of string parsing
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdio.h>      // sscanf
#include <ctype.h>      // isspace

/*============================================================*
 * Parsing double
 *============================================================*/
const char *scanner_ParseDouble(const char *string, double *output) {
    int delta;
    if (sscanf(string, "%lf%n", output, &delta) == 1) {
        // Successfully parsed the double out
        return string + delta;
    }
    return NULL;
}

/*============================================================*
 * Parsing integer
 *============================================================*/
const char *scanner_ParseInteger(const char *string, int *output) {
    int delta;
    if (sscanf(string, "%d%n", output, &delta) == 1) {
        // Successfully parsed the integer out
        return string + delta;
    }
    return NULL;
}

/*============================================================*
 * Parsing string
 *============================================================*/
const char *scanner_ParseString(const char *string, size_t size, char *output) {
    char *where = (char *)string;
    size_t i = 0;
    while (*where && i < size-1 && !isspace(*where)) {
        *output++ = *where++;
        i++;
    }
    if (i > 0) {
        // Don't forget this!
        *output = '\0';
        return where;
    }
    return NULL;
}

/*============================================================*
 * Checking if empty
 *============================================================*/
int scanner_IsEmpty(const char *string) {
    char *where = (char *)string;
    while (*where) {
        if (!isspace(*where++)) {
            return 0;
        }
    }
    return 1;
}

/*============================================================*/