#include "tube.h"

#include <stdlib.h>

#include "util.h"

/**
 * Returns if `tube` is empty.
 *
 * @param[in] tube Tube to check
 *
 * @return Is `tube` empty?
 */
static bool
Tube_is_empty(const Tube *tube)
{
    for (int i = tube->num_slots - 1; i >= 0; --i) {
        if (tube->slots[i].color != EMPTY_COLOR_INDEX) {
            return false;
        }
    }
    return true;
}

/**
 * Returns if `tube` is full.
 *
 * @param[in] tube Tube to check
 *
 * @return Is `tube` full?
 */
static bool
Tube_is_full(const Tube *tube)
{
    for (int i = 0; i < tube->num_slots; ++i) {
        if (tube->slots[i].color == EMPTY_COLOR_INDEX) {
            return false;
        }
    }
    return true;
}

Tube *
Tube_create(int num_slots)
{
    Tube *tube = malloc(sizeof *tube);

    tube->num_slots = num_slots;
    tube->slots = malloc(num_slots * sizeof *tube->slots);
    Tube_clear(tube);

    return tube;
}

void
Tube_destroy(Tube *tube)
{
    if (tube == NULL) {
        return;
    }

    free(tube->slots);

    free(tube);
}

void
Tube_clear(Tube *tube)
{
    for (int i = tube->num_slots - 1; i >= 0; --i) {
        tube->slots[i].color = EMPTY_COLOR_INDEX;
        tube->slots[i].is_hidden = false;
    }
}

int
Tube_add_color(Tube *tube, int color)
{
    if (Tube_is_full(tube)) {
        return TUBE_FAILURE;
    }
    for (int i = 0; i < tube->num_slots; ++i) {
        if (tube->slots[i].color == EMPTY_COLOR_INDEX) {
            tube->slots[i].color = color;
            return TUBE_SUCCESS;
        }
    }
    /* We should never get here */
    return TUBE_FAILURE;
}

/**
 * Writes topmost ColorChunk of `tube` to ColorChunk pointed to by `p_chunk`.
 *
 * @param[in] tube Tube to check
 * @param[out] p_chunk pointer to ColorChunk to write result to
 */
static void
Tube_get_top_chunk(const Tube *tube, ColorChunk *p_chunk)
{
    int color = EMPTY_COLOR_INDEX;
    int i = tube->num_slots - 1;
    for (; i >= 0; --i) {
        if (tube->slots[i].color != EMPTY_COLOR_INDEX) {
            color = tube->slots[i].color;
            break;
        }
    }
    int count = 0; /* Index is on upper slot, so counter is increased any way */
    for (; i >= 0; --i) {
        if (tube->slots[i].color != color) {
            break;
        }
        ++count;
    }
    p_chunk->color = color;
    p_chunk->count = count;
}

/**
 * Adds ColorChunk pointed to by `p_chunk` to `tube` either with or without
 * `check`.
 *
 * @param[in] tube Tube to add chunk to
 * @param[in] p_chunk pointer to ColorChunk to add
 * @param[in] check perform check for push?
 *
 * @return Error code
 */
static int
Tube_push_chunk_aux(Tube *tube, const ColorChunk *p_chunk, bool check)
{
    int color = EMPTY_COLOR_INDEX;
    int count = 0;
    int i = tube->num_slots - 1;
    for (; i >= 0; --i) {
        if (tube->slots[i].color != EMPTY_COLOR_INDEX) {
            break;
        }
        ++count;
    }
    if (check == true) {
        if (i >= 0) {
            color = tube->slots[i].color;
        }
        if (color != EMPTY_COLOR_INDEX && p_chunk->color != color) {
            return TUBE_FAILURE;
        }
        if (p_chunk->count > count) {
            return TUBE_FAILURE;
        }
    }
    ++i; /* Counter is on first non-empty slot (or for empty) */
    for (int j = 0; j < p_chunk->count; ++i, ++j) {
        tube->slots[i].color = p_chunk->color;
    }
    return TUBE_SUCCESS;
}

/**
 * Auxiliary function that removes ColorChunk from `tube`. If `p_chunk` is NULL,
 * remove topmost chunk of tube. If not, only remove if it fits `p_chunk`.
 *
 * @param[in] tube Tube to remove chunk from
 * @param[in] p_chunk pointer to ColorChunk to remove (NULL for any)
 */
static void
Tube_pop_chunk_aux(Tube *tube, const ColorChunk *p_chunk)
{
    int color = EMPTY_COLOR_INDEX;
    int i = tube->num_slots - 1;
    for (; i >= 0; --i) {
        if (tube->slots[i].color != EMPTY_COLOR_INDEX) {
            color = tube->slots[i].color;
            break;
        }
    }
    if (p_chunk != NULL) {
        for (int j = 0; j < p_chunk->count; --i, ++j) {
            if (tube->slots[i].color != p_chunk->color) {
                break;
            }
            tube->slots[i].color = EMPTY_COLOR_INDEX;
        }
    } else {
        for (; i >= 0; --i) {
            if (tube->slots[i].color != color) {
                break;
            }
            tube->slots[i].color = EMPTY_COLOR_INDEX;
        }
    }
}

/**
 * Adds ColorChunk pointed to by `p_chunk` to `tube` (with check).
 *
 * @param[in] tube Tube to add chunk to
 * @param[in] p_chunk pointer to ColorChunk to add
 *
 * @return Error code
 */
static inline int
Tube_push_chunk(Tube *tube, const ColorChunk *p_chunk)
{
    return Tube_push_chunk_aux(tube, p_chunk, true);
}

/**
 * Removes topmost ColorChunk from `tube`.
 *
 * @param[in] tube Tube to remove topmost chunk from
 */
static inline void
Tube_pop_chunk(Tube *tube)
{
    Tube_pop_chunk_aux(tube, NULL);
}

int
Tube_pour(Tube *tube_src, Tube *tube_dst, ColorChunk *p_chunk)
{
    if (Tube_is_empty(tube_src) || Tube_is_full(tube_dst)) {
        return TUBE_FAILURE;
    }
    ColorChunk chunk = {0};
    Tube_get_top_chunk(tube_src, &chunk);
    if (Tube_push_chunk(tube_dst, &chunk) == TUBE_FAILURE) {
        return TUBE_FAILURE;
    }
    Tube_pop_chunk(tube_src);
    if (p_chunk != NULL) {
        *p_chunk = chunk;
    }
    return TUBE_SUCCESS;
}

/**
 * Adds ColorChunk pointed to by `p_chunk` to `tube` (without check).
 *
 * @param[in] tube Tube to add chunk to
 * @param[in] p_chunk pointer to ColorChunk to add
 */
static void
Tube_add_chunk(Tube *tube, const ColorChunk *p_chunk)
{
    Tube_push_chunk_aux(tube, p_chunk, false);
}

/**
 * Removes ColorChunk pointed to by `p_chunk` from `tube`.
 *
 * @param[in] tube Tube to remove chunk from
 * @param[in] p_chunk pointer to ColorChunk to remove
 */
static void
Tube_remove_chunk(Tube *tube, const ColorChunk *p_chunk)
{
    Tube_pop_chunk_aux(tube, p_chunk);
}

/**
 * We assume everything went smoothly so we don't need checks
 */
void
Tube_revert(Tube *tube_src, Tube *tube_dst, const ColorChunk *p_chunk)
{
    Tube_remove_chunk(tube_dst, p_chunk);
    Tube_add_chunk(tube_src, p_chunk);
}

bool
Tube_is_pure(const Tube *tube)
{
    const int color = tube->slots[0].color;
    for (int i = 1; i < tube->num_slots; ++i) {
        if (tube->slots[i].color != color) {
            return false;
        }
    }
    return true;
}

bool
Tube_is_one_color(const Tube *tube)
{
    const int color = tube->slots[0].color;
    for (int i = 1; i < tube->num_slots; ++i) {
        if (tube->slots[i].color == EMPTY_COLOR_INDEX) {
            continue;
        }
        if (tube->slots[i].color != color) {
            return false;
        }
    }
    return true;
}
