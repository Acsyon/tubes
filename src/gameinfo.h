/** gameinfo.h
 *
 * Header for main game logic of 'tubes'.
 */

#ifndef GAMEINFO_H_INCLUDED
#define GAMEINFO_H_INCLUDED

#include <stdbool.h>
#include <stdio.h>

#include "log.h"

/**
 * Struct for general game information and state.
 */
typedef struct {
    int num_tubes;
    int num_extra;
    Tube **tubes;
    unsigned int seed;
    const char *filename;
} GameInfo;

/**
 * Generates GameInfo object with `num_colors` colors, `num_extra` extra
 * tubes and `num_slots` slots per tube from seed `seed`.
 *
 * @param[in] num_colors number of colors for game
 * @param[in] num_extra number of extra tubes for game
 * @param[in] num_slots number of slots for game
 * @param[in] seed seed for game
 *
 * @return Pointer to newly allocated and initialized GameInfo object
 */
GameInfo *
GameInfo_create_from_seed(
  int num_colors, int num_extra, int num_slots, int seed
);

/**
 * Reads GameInfo object from `filename`.
 *
 * @param[in] filename input filename to read data from
 *
 * @return Pointer to newly allocated and initialized GameInfo object
 */
GameInfo *
GameInfo_create_from_file(const char *filename);

/**
 * Destroys `info` and frees memory.
 */
void
GameInfo_destroy(GameInfo *info);

/**
 * Runs main game loop on `info`.
 *
 * @param[in] info GameInfo object to perform action on
 */
void
GameInfo_play(GameInfo *info);

/**
 * Tries to solve game in `info`. If successful, writes solution to file with a
 * standardize name (either "${info->seed}.solution" or
 * "${info->filename}.solution").
 *
 * @param[in] info GameInfo object to perform action on
 */
void
GameInfo_solve(GameInfo *info);

#endif /* GAMEINFO_H_INCLUDED */
