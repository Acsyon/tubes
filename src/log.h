/** log.h
 *
 * Header for log of actions of 'tubes'. Needed to revert moves and for solver.
 */

#ifndef LOG_H_INCLUDED
#define LOG_H_INCLUDED

#include <stdio.h>

#include "tube.h"

/**
 * Auxiliary struct to store action.
 */
typedef struct {
    int i_src;
    int i_dst;
    ColorChunk chunk;
} Action;

/**
 * Struct for action log.
 */
typedef struct {
    Action *actions;
    int counter;
    int capacity;
} ActionLog;

/**
 * Allocates and initializes ActionLog object.
 *
 * @return Pointer newly allocated and initialized ActionLog object
 */
ActionLog *
ActionLog_create(void);

/**
 * Destroys `log` and frees memory.
 *
 * @param[in] log ActionLog to be freed
 */
void
ActionLog_destroy(ActionLog *log);

/**
 * Duplicates `log`.
 *
 * @param[in] log ActionLog to be duplicated
 *
 * @return Pointer newly allocated copy of `log`
 */
ActionLog *
ActionLog_duplicate(const ActionLog *log);

/**
 * Appends `action` to the end of `log` and potentially increases capacity.
 *
 * @param[in] log ActionLog to be appended
 * @param[in] action Action to append
 *
 * @return Error code
 */
void
ActionLog_push_back(ActionLog *log, const Action *action);

/**
 * Retrieves last Action of `log` and writes it to `action`.
 *
 * @param[in] log ActionLog to pop
 * @param[out] action popped action
 *
 * @return Error code
 */
int
ActionLog_pop(ActionLog *log, Action *action);

/**
 * Prints contents of `log` to `out` in standardized way.
 *
 * @param[in] out output FILE stream
 * @param[in] log ActionLog to be printed
 */
void
ActionLog_fprint(FILE *out, const ActionLog *log);

#endif /* LOG_H_INCLUDED */
