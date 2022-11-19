/** util.h
 *
 * Utility header for 'tubes'.
 */

#ifndef UTIL_H_INCLUDED
#define UTIL_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>

#define EMPTY_COLOR_INDEX -1

/**
 * Error codes/return values of functions.
 */
enum {
    TUBE_FAILURE = -1,
    TUBE_SUCCESS = 0,
};

/**
 * Variadic error MACRO. Prints formatted message to 'stderr' and exits.
 *
 * @param[in] __VA_ARGS__ variadic arguments for (f)printf
 */
#define ERROR(...)                                                             \
    do {                                                                       \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(EXIT_FAILURE);                                                    \
    } while (0)

#endif /* UTIL_H_INCLUDED */
