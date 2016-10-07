/**********************************************************//**
 * @file scanner.h
 * @brief Header file for string parsing
 * @author Alec Shinomiya
 **************************************************************/

#ifndef _SCANNER_H_
#define _SCANNER_H_

/**********************************************************//**
 * @brief Get a double from the string
 * @param string: The characters to read
 * @param output: The location to put the parsed double
 * @return NULL or pointer to rest of string
 **************************************************************/
extern const char *scanner_ParseDouble(const char *string, double *output);

/**********************************************************//**
 * @brief Get an integer from the string
 * @param string: The characters to read
 * @param output: The location to put the parsed int
 * @return NULL or pointer to rest of string
 **************************************************************/
extern const char *scanner_ParseInteger(const char *string, int *output);

/**********************************************************//**
 * @brief Get a word (with no whitespace) from the string
 * @param string: The characters to read
 * @param size: The size of the output array
 * @param output: The location to put the parsed word
 * @return NULL or pointer to rest of string
 **************************************************************/
extern const char *scanner_ParseString(const char *string, size_t size, char *output);

/**********************************************************//**
 * @brief Check if the string contains only whitespace
 * @param string: The characters to check
 * @return 1 if it is empty else 0
 **************************************************************/
extern int scanner_IsEmpty(const char *string);

/*============================================================*/
#endif // _SCANNER_H_