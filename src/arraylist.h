/**********************************************************//**
 * @file arraylist.h
 * @brief Header file for ARRAYLIST data structure
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _ARRAYLIST_H_
#define _ARRAYLIST_H_

// Standard library
#include <stdlib.h>     // size_t

/**********************************************************//**
 * @struct ARRAYLIST
 * @brief Struct containing variable length array of any
 * data type.
 **************************************************************/
typedef struct {
    size_t maxlength;   ///< The actual size of the data block in item units
    size_t length;      ///< The number of items in the list
    size_t itemsize;    ///< The size of each item unit
    void *data;         ///< The actual data in the list
} ARRAYLIST;

/**********************************************************//**
 * @brief Create an empty arraylist with the given capacity
 * @param list: The arraylist to initialize
 * @param itemsize: The size of each element of the arraylist
 * @param maxlength: The capacity of the arraylist
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int arraylist_Create(ARRAYLIST *list, size_t itemsize, size_t maxlength);

/**********************************************************//**
 * @brief Get the number of items in the arraylist
 * @param list: The arraylist to check
 * @return The number of items in the list
 **************************************************************/
extern size_t arraylist_Length(const ARRAYLIST *list);

/**********************************************************//**
 * @brief Get a pointer to the given item in the arraylist
 * @param list: The arraylist to check
 * @param index: The index of the item from 0 to the length-1
 * @return Pointer to the item or NULL on failure
 **************************************************************/
extern void *arraylist_At(ARRAYLIST *list, int index);

/**********************************************************//**
 * @brief Append the new item to the end of the arraylist.
 * This function may grow the arraylist.
 * @param list: The arraylist to append to
 * @param item: The new data to copy in to the list. The data
 * can be deleted afterward, although this will not perform
 * a deep copy of any allocated regions.
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int arraylist_Append(ARRAYLIST *list, const void *item);

/**********************************************************//**
 * @brief Reduce the capacity of the arraylist to the length
 * in order to save memory usage.
 * @param list: The arraylist to compress
 * @return SUCCESS or FAILURE
 **************************************************************/
extern int arraylist_Compress(ARRAYLIST *list);

/**********************************************************//**
 * @brief Convert the arraylist to a normal array. If the
 * ARRAYLIST is deleted the return value of this function will
 * be freed.
 * @param list: The arraylist to read
 * @return Pointer to the data array or NULL on failure
 **************************************************************/
extern void *arraylist_GetData(ARRAYLIST *list);

/**********************************************************//**
 * @brief Destroy the arraylist and free allocated data.
 * @param list: The arraylist to destroy
 **************************************************************/
extern void arraylist_Destroy(ARRAYLIST *list);

/*============================================================*/
#endif // _ARRAYLIST_H_
