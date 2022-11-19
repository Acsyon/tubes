#include "log.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "gameinfo.h"
#include "util.h"

#define ACTION_LOG_INITIAL_CAPACITY 64

ActionLog *
ActionLog_create(void)
{
    ActionLog *log = malloc(sizeof *log);

    log->counter = 0;
    log->capacity = ACTION_LOG_INITIAL_CAPACITY;
    log->actions = malloc(log->capacity * sizeof *log->actions);

    return log;
}

void
ActionLog_destroy(ActionLog *log)
{
    if (log == NULL) {
        return;
    }

    free(log->actions);

    free(log);
}

ActionLog *
ActionLog_duplicate(const ActionLog *log)
{
    ActionLog *dup = malloc(sizeof *log);

    dup->counter = log->counter;
    dup->capacity = log->capacity;
    dup->actions = malloc(dup->capacity * sizeof *dup->actions);
    dup->actions = memcpy(
      dup->actions, log->actions, log->capacity * sizeof *log->actions
    );

    return dup;
}

void
ActionLog_push_back(ActionLog *log, const Action *action)
{
    if (log->counter == log->capacity) {
        log->capacity *= 2;
        log->actions
          = realloc(log->actions, log->capacity * sizeof *log->actions);
    }
    log->actions[log->counter] = *action;
    ++log->counter;
}

int
ActionLog_pop(ActionLog *log, Action *action)
{
    if (log->counter == 0) {
        return TUBE_FAILURE;
    }
    --log->counter;
    *action = log->actions[log->counter];
    return TUBE_SUCCESS;
}

void
ActionLog_fprint(FILE *out, const ActionLog *log)
{
    const int counter_width = (int) log10(log->counter + 1) + 1;
    for (int i = 0; i < log->counter; ++i) {
        const int i_src = log->actions[i].i_src + 1;
        const int i_dst = log->actions[i].i_dst + 1;
        fprintf(out, "%*i: %2i %2i\n", counter_width, i + 1, i_src, i_dst);
    }
}
