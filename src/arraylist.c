/**********************************************************//**
 * @file arraylist.c
 * @brief Header file for ARRAYLIST data structure
 * @author Alec Shinomiya
 **************************************************************/

// Standard library
#include <stdlib.h>     // size_t, malloc, free
#include <stdio.h>      // stderr, fprintf
#include <string.h>     // memcpy

// This project
#include "macro.h"      // SUCCESS, FAILURE
#include "arraylist.h"  // ARRAYLIST

// Debugging modules
#include "debug.h"

/*============================================================*
 * Creation function
 *============================================================*/
int arraylist_Create(ARRAYLIST *list, size_t itemsize, int maxlength) {
    // Error checking
    if (itemsize <= 0) {
        errmsg("Invalid item size\n");
        return FAILURE;
    }
    if (maxlength <= 0) {
        errmsg("Invalid capacity\n");
        return FAILURE;
    }
    
    // Set up properties
    list->maxlength = maxlength;
    list->itemsize = itemsize;
    list->length = 0;
    
    // Set up data array
    list->data = malloc(itemsize * maxlength);
    if (!list->data) {
        errmsg("Out of memory\n");
        return FAILURE;
    }
    return SUCCESS;
}

/*============================================================*
 * Getter
 *============================================================*/
int arraylist_Length(const ARRAYLIST *list) {
    return (int)list->length;
}

/*============================================================*
 * Item getter
 *============================================================*/
void *arraylist_At(ARRAYLIST *list, int index) {
    // Perform pointer arithmetic
    char *temp = list->data;
    if (sizeof(char) != 1) {
        errmsg("sizeof(char) is %d on this system\n", (int)sizeof(char));
        return NULL;
    }
    
    if (index < 0 || index >= list->maxlength) {
        errmsg("Index %d out of bounds\n", index);
        return NULL;
    }
    
    temp += index * list->itemsize;
    return (void *)temp;
}

/*============================================================*
 * Iterator
 *============================================================*/
void *arraylist_Begin(ARRAYLIST *list) {
    return arraylist_At(list, 0);
}

void *arraylist_End(ARRAYLIST *list) {
    int index = list->length - 1;
    if (index < 0) {
        return NULL;
    }
    return arraylist_At(list, index);
}

/*============================================================*
 * Appending function
 *============================================================*/
int arraylist_Append(ARRAYLIST *list, const void *item) {
    // Grow the arraylist if it is too small
    if (list->length >= list->maxlength) {
        void *new = malloc(list->itemsize * list->maxlength * 2);
        if (!new) {
            errmsg("Out of memory\n");
            return FAILURE;
        }
        memcpy(new, list->data, list->length * list->itemsize);
        free(list->data);
        list->data = new;
        list->maxlength *= 2;
    }
    
    // Copy the new item into the arraylist
    void *destination = arraylist_At(list, list->length);
    if (!destination) {
        errmsg("Unable to offset into data field\n");
        return FAILURE;
    }
    memcpy(destination, item, list->itemsize);
    list->length++;
    return SUCCESS;
}

/*============================================================*
 * Suze reduction
 *============================================================*/
int arraylist_Compress(ARRAYLIST *list) {
    // Tries to reduce size usage
    void *new = malloc(list->itemsize * list->length);
    if (!new) {
        errmsg("Out of memory\n");
        return FAILURE;
    }
    memcpy(new, list->data, list->length * list->itemsize);
    free(list->data);
    list->data = new;
    return SUCCESS;
}

/*============================================================*
 * Convert to normal array
 *============================================================*/
void *arraylist_GetData(ARRAYLIST *list) {
    // Get all the data out
    return list->data;
}

/*============================================================*
 * Destructor
 *============================================================*/
void arraylist_Destroy(ARRAYLIST *list) {
    if (list->data) {
        free(list->data);
        list->data = NULL;
    }
    return;
}

/*============================================================*/
