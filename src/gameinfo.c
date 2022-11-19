#include "gameinfo.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "input.h"
#include "log.h"
#include "tube.h"
#include "util.h"

#define USER_INPUT_BUFFER_SIZE 16

/**
 * Auxiliary struct for color pool.
 */
typedef struct {
    int num_colors;
    int *data;
    int num_slots;
} ColorPool;

/**
 * Allocates and initializes full ColorPool object of `num_colors` colors and
 * `num_slots` slots.
 *
 * @return Pointer to newly allocated ColorPool object
 */
static ColorPool *
ColorPool_create_full(int num_colors, int num_slots)
{
    ColorPool *pool = malloc(sizeof *pool);

    pool->num_colors = num_colors;
    pool->data = malloc(num_colors * sizeof *pool->data);
    pool->num_slots = num_slots;
    for (int i = 0; i < num_colors; ++i) {
        pool->data[i] = num_slots;
    }

    return pool;
}

/**
 * Destroys `pool` and frees memory.
 *
 * @param[in] pool ColorPool to be freed
 */
static void
ColorPool_destroy(ColorPool *pool)
{
    if (pool == NULL) {
        return;
    }

    free(pool->data);

    free(pool);
}

/**
 * Returns if `pool` is empty.
 *
 * @param[in] pool ColorPool to be checked
 *
 * @return Is `pool` empty?
 */
static bool
ColorPool_is_empty(const ColorPool *pool)
{
    for (int i = 0; i < pool->num_colors; ++i) {
        if (pool->data[i] != 0) {
            return false;
        }
    }
    return true;
}

/**
 * Randomly picks and removes color from `pool` and returns it.
 *
 * @param[in] pool ColorPool to pick color from
 *
 * @return Randomly picked color
 */
static int
ColorPool_pick_color(ColorPool *pool)
{
    int color = EMPTY_COLOR_INDEX;
    do {
        color = rand() % pool->num_colors;
    } while (pool->data[color] == 0);
    --pool->data[color];
    return color;
}

/**
 * Creates GameInfo object with `num_colors` colors, `num_extra` extra
 * tubes and `num_slots`.
 *
 * @param[in] num_colors number of colors for game
 * @param[in] num_extra number of extra tubes for game
 * @param[in] num_slots number of slots for game
 *
 * @return Pointer to newly allocated GameInfo object
 */
static GameInfo *
GameInfo_create(int num_colors, int num_extra, int num_slots)
{
    GameInfo *info = malloc(sizeof *info);

    info->num_tubes = num_colors + num_extra;
    info->num_extra = num_extra;
    info->tubes = malloc(info->num_tubes * sizeof *info->tubes);
    info->seed = 0;
    info->filename = NULL;

    for (int i = 0; i < info->num_tubes; ++i) {
        info->tubes[i] = Tube_create(num_slots);
    }

    return info;
}

GameInfo *
GameInfo_create_from_seed(
  int num_colors, int num_extra, int num_slots, int seed
)
{
    GameInfo *info = GameInfo_create(num_colors, num_extra, num_slots);

    info->seed = seed;
    srand(info->seed);

    ColorPool *pool = ColorPool_create_full(num_colors, num_slots);
    while (ColorPool_is_empty(pool) == false) {
        const int color = ColorPool_pick_color(pool);
        Tube *tube = NULL;
        do {
            const int i_tube = rand() % num_colors;
            tube = info->tubes[i_tube];
        } while (Tube_add_color(tube, color) != TUBE_SUCCESS);
    }
    ColorPool_destroy(pool);

    return info;
}

GameInfo *
GameInfo_create_from_file(const char *filename)
{
    Input *input = Input_read(filename);
    if (input == NULL) {
        return NULL;
    }

    const int num_tubes = input->num_tubes;
    const int num_colors = input->num_colors;
    const int num_slots = input->num_slots;
    const int num_extra = num_tubes - num_colors;

    GameInfo *info = GameInfo_create(num_colors, num_extra, num_slots);

    info->filename = filename;

    for (int i_tube = 0; i_tube < num_tubes; ++i_tube) {
        Tube *const tube = info->tubes[i_tube];
        for (int i_slot = 0; i_slot < num_slots; ++i_slot) {
            const int color = input->data[i_tube * num_slots + i_slot];
            Tube_add_color(tube, color);
        }
    }

    Input_destroy(input);

    return info;
}

void
GameInfo_destroy(GameInfo *info)
{
    if (info == NULL) {
        return;
    }

    for (int i = 0; i < info->num_tubes; ++i) {
        Tube_destroy(info->tubes[i]);
    }
    free(info->tubes);

    free(info);
}

/**
 * Prints `info` to FILE stream `out` in a standardized way.
 *
 * @param[in] out output FILE stream
 * @param[in] info GameInfo object to be printed
 */
static void
GameInfo_fprint(FILE *out, const GameInfo *info)
{
    const int num_colors = info->num_tubes - info->num_extra;
    const int tube_width = (int) log10(info->num_tubes + 1) + 1;
    const int color_width = (int) log10(num_colors) + 2;
    for (int i_tube = 0; i_tube < info->num_tubes; ++i_tube) {
        const Tube *const tube = info->tubes[i_tube];
        fprintf(out, "%*i: ", tube_width, i_tube + 1);
        for (int i_slot = 0; i_slot < tube->num_slots; ++i_slot) {
            const TubeSlot *const slot = &tube->slots[i_slot];
            if (slot->is_hidden) {
                fprintf(out, " %*c", color_width, '?');
            } else {
                fprintf(out, "% *i", color_width, slot->color);
            }
            if (i_slot < tube->num_slots - 1) {
                fprintf(out, ", ");
            }
        }
        fprintf(out, "\n");
    }
}

/**
 * Returns if `info` is solved (all tubes are uniformly filled).
 *
 * @param[in] info GameInfo object to be checked
 *
 * @return Is `info` solved?
 */
static bool
GameInfo_is_solved(const GameInfo *info)
{
    for (int i = 0; i < info->num_tubes; ++i) {
        if (Tube_is_pure(info->tubes[i]) == false) {
            return false;
        }
    }
    return true;
}

/**
 * Tries to pour contents of tube with index `i_src` to tube with index `i_dst`
 * (both in `info`) and writes action to `log` if successful.
 *
 * @param[in] info GameInfo object to perform action on
 * @param[in] i_src index of source tube
 * @param[in] i_dst index of destination tube
 * @param[in,out] log ActionLog used to log action
 *
 * @return Error code
 */
static int
GameInfo_pour(GameInfo *info, int i_src, int i_dst, ActionLog *log)
{
    if (i_src >= info->num_tubes || i_dst >= info->num_tubes) {
        return TUBE_FAILURE;
    }
    Tube *const tube_src = info->tubes[i_src];
    Tube *const tube_dst = info->tubes[i_dst];
    Action action = {.i_src = i_src, i_dst = i_dst};
    if (Tube_pour(tube_src, tube_dst, &action.chunk) != TUBE_SUCCESS) {
        return TUBE_FAILURE;
    }
    ActionLog_push_back(log, &action);
    return TUBE_SUCCESS;
}

/**
 * Reverts last action of `info` according to `log`. Also removes this action
 * from `log`.
 *
 * @param[in] info GameInfo object to perform action on
 * @param[in,out] log ActionLog to use to revert last action
 *
 * @return error code
 */
static int
GameInfo_revert_one(GameInfo *info, ActionLog *log)
{
    Action action;
    if (ActionLog_pop(log, &action) != TUBE_SUCCESS) {
        return TUBE_FAILURE;
    }
    Tube *const tube_src = info->tubes[action.i_src];
    Tube *const tube_dst = info->tubes[action.i_dst];
    const ColorChunk *const p_chunk = &action.chunk;
    Tube_revert(tube_src, tube_dst, p_chunk);
    return TUBE_SUCCESS;
}

/**
 * Reverts all actions of `info` according to `log`. Thus, also empties `log`.
 *
 * @param[in] info GameInfo object to perform action on
 * @param[in,out] log ActionLog to use to revert all actions
 */
void
GameInfo_revert_all(GameInfo *info, ActionLog *log)
{
    while (GameInfo_revert_one(info, log) == TUBE_SUCCESS)
        ;
}

/**
 * Reads the first `maxlen` bytes to `buffer` and flushes the rest of the input
 * stream.
 *
 * Since this is C99, we don't have anything fancy like 'sscanf_s' (even C11
 * isn't guaranteed to have that) and again have to do things ourselves.
 *
 * @param[out] buffer char buffer to write input to
 * @param[in] maxlen maximum number of bytes to read
 *
 * @return Number of bytes read
 */
static size_t
_read_stdin(char *buffer, size_t maxlen)
{
    size_t res = maxlen;
    for (size_t i = 0; i < maxlen; ++i) {
        const char c = getchar();
        if (c == '\n' || c == EOF) {
            maxlen = i;
            break;
        }
        buffer[i] = c;
    }
    if (res == maxlen) {
        while (getchar() != '\n' && getchar() != EOF)
            ;
    }
    return res;
}

/**
 * Input action enumerator.
 */
enum {
    INPUT_VALID,
    INPUT_INVALID,
    INPUT_REVERT,
    INPUT_QUIT,
};

/**
 * Parses user input (on 'stdin'). If input was valid move, write indices of
 * tubes to `p_i_src` and `p_i_dst`.
 *
 * @param[out] p_i_src pointer to index of source tube
 * @param[out] p_i_dst pointer to index of destination tube
 *
 * @return Input action enumerator
 */
static int
_parse_input(int *p_i_src, int *p_i_dst)
{
    char buffer[USER_INPUT_BUFFER_SIZE + 1] = {0};
    const size_t len = _read_stdin(buffer, USER_INPUT_BUFFER_SIZE);
    for (size_t i = 0; i < len; ++i) {
        const char c = buffer[i];
        if (isalnum(c) == 0) {
            buffer[i] = ' ';
            continue;
        }
        if (isdigit(c) != 0) {
            buffer[i] = c;
            continue;
        }
        switch (tolower(c)) {
        case 'q':
            return INPUT_QUIT;
        case 'r':
            return INPUT_REVERT;
        default:
            return INPUT_INVALID;
        }
    }
    char *end;
    char *p = buffer;
    *p_i_src = strtol(p, &end, 10) - 1;
    if (p == end) {
        return INPUT_INVALID;
    }
    p = end;
    *p_i_dst = strtol(p, &end, 10) - 1;
    if (p == end) {
        return INPUT_INVALID;
    }
    return INPUT_VALID;
}

void
GameInfo_play(GameInfo *info)
{
    if (info == NULL) {
        return;
    }

    GameInfo_fprint(stdout, info);
    printf("\n");

    ActionLog *log = ActionLog_create();
    int i_src, i_dst;
    for (;;) {
        printf("Src and dst tube: ");
        const int input = _parse_input(&i_src, &i_dst);
        if (input == INPUT_QUIT) {
            break;
        }
        switch (input) {
        case INPUT_VALID:
            GameInfo_pour(info, i_src, i_dst, log);
            break;
        case INPUT_REVERT:
            GameInfo_revert_one(info, log);
            break;
        case INPUT_INVALID:
            continue;
        }
        GameInfo_fprint(stdout, info);
        printf("\n");
        if (GameInfo_is_solved(info) == true) {
            printf("Conglaturation!\n");
            break;
        }
    }
    ActionLog_destroy(log);
}

/**
 * Checks if pouring content of tube with index `i_src` to tube with index
 * `i_dst` (both in `info`) is pointless (uniform tube to empty tube, i.e., does
 * not change situation).
 *
 * @param[in] info GameInfo object to perform action on
 * @param[in] i_src index of source tube
 * @param[in] i_dst index of destination tube
 *
 * @return Error code
 */
static bool
GameInfo_pour_is_pointless(const GameInfo *info, int i_src, int i_dst)
{
    const Tube *const tube_src = info->tubes[i_src];
    const Tube *const tube_dst = info->tubes[i_dst];
    if (Tube_is_one_color(tube_src) == true && Tube_is_pure(tube_dst) == true) {
        return true;
    }
    return false;
}

/**
 * Loops over destination tubes for naive backtracking solver of `info` with
 * ActionLog `log`.
 *
 * @param[in] info GameInfo object to check for solution
 * @param[out] log ActionLog to write to work with
 * @param[in] i_src index of source tube
 *
 * @return Error code of pouring try
 */
static int
GameInfo_solver_loop_dst(GameInfo *info, ActionLog *log, int i_src)
{
    for (int i_dst = 0; i_dst < info->num_tubes; ++i_dst) {
        if (i_dst == i_src) {
            continue;
        }
        if (GameInfo_pour_is_pointless(info, i_src, i_dst) == true) {
            continue;
        }
        if (GameInfo_pour(info, i_src, i_dst, log) == TUBE_SUCCESS) {
            return TUBE_SUCCESS;
        }
    }
    return TUBE_FAILURE;
}

/**
 * Loops over source tubes for naive backtracking solver of `info` with
 * ActionLog `log`.
 *
 * @param[in] info GameInfo object to check for solution
 * @param[out] log ActionLog to write to work with
 *
 * @return Found solution?
 */
static bool
GameInfo_solver_loop_src(GameInfo *info, ActionLog *log)
{
    for (int i_src = 0; i_src < info->num_tubes; ++i_src) {
        if (Tube_is_pure(info->tubes[i_src]) == true) {
            continue;
        }
        if (GameInfo_solver_loop_dst(info, log, i_src) == TUBE_SUCCESS) {
            if (GameInfo_is_solved(info) == true) {
                return true;
            }
            if (GameInfo_solver_loop_src(info, log) == true) {
                return true;
            }
            GameInfo_revert_one(info, log);
        }
    }
    return false;
}

/**
 * Returns if GameInfo object `info` is solvable and writes first found solution
 * to `log` (if not NULL).
 *
 * @param[in] info GameInfo object to check for solution
 * @param[out] log ActionLog to write solution to (if found)
 *
 * @return Found solution?
 */
static bool
GameInfo_find_solution(GameInfo *info, ActionLog *log)
{
    ActionLog *auxlog = ActionLog_create();
    const bool res = GameInfo_solver_loop_src(info, auxlog);
    if (log != NULL && res == true) {
        ActionLog tmp = *log;
        *log = *auxlog;
        *auxlog = tmp;
    }
    ActionLog_destroy(auxlog);
    return res;
}

/**
 * Generates output file of solution according to `info`, either
 * "${info->seed}.solution" or "${info->filename}.solution".
 *
 * @param[in] info GameInfo object to generate output FILE from
 *
 * @return FILE stream for solution output file
 */
static FILE *
GameInfo_solution_file(const GameInfo *info)
{
    char *filename = NULL;
    if (info->filename == NULL) {
        const size_t len = (size_t) log10(info->seed) + 1;
        const size_t maxlen = len + sizeof "seed.solution";
        filename = malloc(maxlen * sizeof *filename);
        snprintf(filename, maxlen, "seed%u.solution", info->seed);
    } else {
        const size_t len = strlen(info->filename);
        const size_t maxlen = len + sizeof ".solution";
        filename = malloc(maxlen * sizeof *filename);
        snprintf(filename, maxlen, "%s.solution", info->filename);
    }
    FILE *out = fopen(filename, "w");
    free(filename);
    return out;
}

void
GameInfo_solve(GameInfo *info)
{
    if (info == NULL) {
        return;
    }

    ActionLog *log = ActionLog_create();
    if (GameInfo_find_solution(info, log) == true) {
        ActionLog *duplog = ActionLog_duplicate(log);
        GameInfo_revert_all(info, duplog);
        ActionLog_destroy(duplog);

        FILE *out = GameInfo_solution_file(info);
        GameInfo_fprint(out, info);
        fprintf(out, "\n");
        ActionLog_fprint(out, log);
        fclose(out);
    }
    ActionLog_destroy(log);
}
