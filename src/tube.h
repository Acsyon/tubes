/** tube.h
 *
 * Header for tube object of 'tubes'. Basically, contains central game logic.
 */

#ifndef TUBE_H_INCLUDED
#define TUBE_H_INCLUDED

#include <stdbool.h>

/**
 * Auxiliary struct for color chunk to pour.
 */
typedef struct {
    int color;
    int count;
} ColorChunk;

/**
 * Auxiliary struct for slot of tube.
 */
typedef struct {
    int color;
    bool is_hidden;
} TubeSlot;

/**
 * Struct for tube.
 */
typedef struct {
    int num_slots;
    TubeSlot *slots;
} Tube;

/**
 * Creates clear Tube with `num_slots` slots.
 *
 * @param[in] num_slots number of slots
 *
 * @return newly malloc'd and initialized Tube object
 */
Tube *
Tube_create(int num_slots);

/**
 * Destroys `tube` and frees memory.
 *
 * @param[in] tube Tube to be destroyed
 */
void
Tube_destroy(Tube *tube);

/**
 * Clears/empties `tube` (fills it with EMPTY_SLOT_INDEX).
 *
 * @param[in] tube Tube to be cleared
 */
void
Tube_clear(Tube *tube);

/**
 * Adds single slot on top of `tube` (does not have to fit) (for initialization
 * of game).
 *
 * @param[in] tube Tube to add color to
 * @param[in] color color to add
 *
 * @return Error code
 */
int
Tube_add_color(Tube *tube, int color);

/**
 * Tries to pour contents of `tube_src` to `tube_dst` and writes moved chunk to
 * `p_chunk_log` if successful.
 *
 * @param[in] tube_src source tube
 * @param[in] tube_dst destination tube
 * @param[out] p_chunk_log pointer to moved ColorChunk
 *
 * @return Error code
 */
int
Tube_pour(Tube *tube_src, Tube *tube_dst, ColorChunk *p_chunk_log);

/**
 * Reverts pouring of `p_chunk` from `tube_src` to `tube_dst` (without
 * additional checks).
 *
 * @param[in] tube_src original source tube
 * @param[in] tube_dst original destination tube
 * @param[out] p_chunk_log pointer to moved ColorChunk to revert
 */
void
Tube_revert(Tube *tube_src, Tube *tube_dst, const ColorChunk *p_chunk);

/**
 * Returns if `tube` is pure (all slots have same color).
 *
 * @param[in] tube Tube to check
 *
 * @return Is `tube` pure?
 */
bool
Tube_is_pure(const Tube *tube);

/**
 * Returns if `tube` is one color (even if not all slots are filled).
 *
 * @param[in] tube Tube to check
 *
 * @return Is `tube` only one color?
 */
bool
Tube_is_one_color(const Tube *tube);

#endif /* TUBE_H_INCLUDED */
