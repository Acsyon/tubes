/** input.h
 *
 * Header for handling input from files (i.e., reading games from files) for
 * 'tubes'. Probably the most dubious part of the project...
 */

#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

/**
 * Struct for input data.
 */
typedef struct {
    int num_tubes;
    int num_colors;
    int num_slots;
    int *data;
} Input;

/**
 * Creates Input object from data in `filename`.
 *
 * @param[in] filename input filename
 *
 * @return newly malloc'd and initialized Input object
 */
Input *
Input_read(const char *filename);

/**
 * Destroys `input` and frees memory.
 *
 * @param[in] input Input to be destroyed
 */
void
Input_destroy(Input *input);

#endif /* INPUT_H_INCLUDED */
