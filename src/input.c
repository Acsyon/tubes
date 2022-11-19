#include "input.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define BUFFER_SIZE 128
#define DEFAULT_INITIAL_CAPACITY 4

/**
 * Poor man's version of an std::vector<int> and failed attempt of compulsive
 * OOP.
 */
typedef struct {
    int size;
    int capacity;
    int *data;
} IntVec;

/**
 * Creates IntVec object with an initial capacity of `initial_capacity`.
 *
 * @param[in] initial_capacity initial capacity of vector
 *
 * @return Pointer to newly allocated IntVec object
 */
static IntVec *
IntVec_alloc(int initial_capacity)
{
    IntVec *vec = malloc(sizeof *vec);

    if (initial_capacity <= 0) {
        initial_capacity = DEFAULT_INITIAL_CAPACITY;
    }

    vec->size = 0;
    vec->capacity = initial_capacity;
    vec->data = malloc(vec->capacity * sizeof *vec->data);

    return vec;
}

/**
 * Destroys `vec` and frees memory.
 *
 * @param[in] vec IntVec to be destroyed
 */
static void
IntVec_free(IntVec *vec)
{
    if (vec == NULL) {
        return;
    }

    free(vec->data);

    free(vec);
}

/**
 * Adds integer `val` to IntVec `vec` and potentially resizes memory.
 *
 * @param[in] vec vector to add to
 * @param[in] val value to add to vector
 */
static void
IntVec_push_back(IntVec *vec, int val)
{
    if (vec->size == vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, vec->capacity * sizeof *vec->data);
    }
    vec->data[vec->size] = val;
    ++vec->size;
}

/**
 * Shrinks memory of `vec` to actual number of elements.
 *
 * @param[in] vec vector to shrink
 */
static void
IntVec_shrink_to_fit(IntVec *vec)
{
    if (vec->size == vec->capacity) {
        return;
    }
    vec->capacity = vec->size;
    vec->data = realloc(vec->data, vec->size * sizeof *vec->data);
}

/**
 * Counts lines (separated by '\n') in file `in`. Rewinds `in`.
 *
 * @param[in] in FILE stream to count lines of
 *
 * @return Number of lines in `in`
 */
static int
_count_lines(FILE *in)
{
    rewind(in);
    char c;
    int ctr = 1;
    while ((c = fgetc(in)) != EOF) {
        if (c == '\n') {
            ++ctr;
        }
    };
    rewind(in);
    return ctr;
}

/**
 * Buffered input function to read data from `in`. Buffer never exceeds certain
 * size.
 *
 * There's probably already an implementation for this which does things better,
 * faster and smarter than I ever could, but I don't care! I do my own
 * unoptimized stuff! Don't judge me!
 *
 * @todo Maybe rewrite/optimize this function. Somehow, I don't like it in this
 * state.
 *
 * @param[in] in input FILE stream to read data from
 * @param[in,out] p_num_elems pointer to number of elements of returned array
 * (output), also used as initial guess for capacity of output vector
 *
 * @return Newly allocated array with elements of line
 */
static int *
_read_elems(FILE *in, int *p_num_elems)
{
    static char buffer[BUFFER_SIZE + 1] = {0};

    IntVec *vec = IntVec_alloc(*p_num_elems);

    bool is_done = false;
    bool is_error = false;
    bool is_empty = true;
    for (;;) {
        fpos_t pos;
        fgetpos(in, &pos);

        /* Preprocess input in buffer */
        int idx;
        is_error = false;
        for (idx = 0; idx < BUFFER_SIZE; ++idx) {
            const char c = fgetc(in);
            if (isdigit(c) || c == '-') {
                buffer[idx] = c;
            } else if (c == '\n' || c == EOF) {
                buffer[idx] = '\0';
                is_done = true;
                break;
            } else if (isalpha(c)) {
                is_error = true;
                break;
            } else if (c == '#') {
                buffer[idx] = '#';
            } else {
                buffer[idx] = ' ';
            }
            if (is_empty == true) {
                if (c == '#') { /* Comment */
                    break;
                }
                if (c != ' ' && c != '\n' && c != '\0' && c != EOF) {
                    is_empty = false;
                }
            }
        }
        if (is_empty == true || is_error == true) {
            break;
        }

        /* Read elements */
        char *endp = buffer;
        const char *nptr = endp;
        do {
            nptr = endp;
            const int elem = (int) strtol(nptr, &endp, 10);
            if (endp != nptr) {
                IntVec_push_back(vec, elem);
            }
        } while (endp != nptr && endp != NULL);

        if (is_done == true) {
            break;
        }

        /* Move to last whitespace so that all numbers are complete */
        long last = idx;
        for (; buffer[last] != ' ' && last > 0; --last)
            ;
        ++last;

        /* Is buffer empty? */
        if (last == 0) {
            continue;
        }

        /* Restart buffer at last whitespace */
        fsetpos(in, &pos);
        fseek(in, last, SEEK_CUR);
    }
    if (is_empty == true) {
        IntVec_free(vec);
        *p_num_elems = 0;
        return NULL;
    }
    if (is_error == true) {
        IntVec_free(vec);
        *p_num_elems = -1;
        return NULL;
    }

    IntVec_shrink_to_fit(vec);
    int *res = vec->data;
    *p_num_elems = vec->size;
    vec->data = NULL;
    IntVec_free(vec);

    return res;
}

/**
 * Auxiliary struct for raw (line by line) input data.
 */
typedef struct {
    int num_lines;
    int num_elems;
    int **data;
} RawInput;

/**
 * Creates RawInput object from data in `filename`.
 *
 * @param[in] filename input filename
 *
 * @return Pointer to newly allocated and initialized RawInput object
 */
static RawInput *
RawInput_read(const char *filename)
{
    FILE *in = fopen(filename, "r");
    if (in == NULL) {
        return NULL;
    }

    RawInput *raw = malloc(sizeof *raw);

    raw->num_lines = _count_lines(in);
    raw->data = malloc(raw->num_lines * sizeof *raw->data);
    raw->num_elems = -1;
    int i_line;
    for (i_line = 0; i_line < raw->num_lines; ++i_line) {
        int num_elems = raw->num_elems;
        raw->data[i_line] = _read_elems(in, &num_elems);
        if (num_elems == -1) {
            ERROR("Error reading file '%s' at line %i!", filename, i_line + 1);
        }
        if (raw->data[i_line] != NULL && raw->num_elems == -1) {
            raw->num_elems = num_elems;
        }
        if (raw->data[i_line] != NULL && raw->num_elems != num_elems) {
            fprintf(
              stderr, "Invalid number of arguments in file '%s' at line %i!\n",
              filename, i_line + 1
            );
            ERROR("Expected %i got %i!", raw->num_elems, num_elems);
        }
    }

    fclose(in);

    return raw;
}

/**
 * Destroys `raw` and frees memory.
 *
 * @param[in] raw RawInput to be destroyed
 */
static void
RawInput_free(RawInput *raw)
{
    if (raw == NULL) {
        return;
    }

    for (int i = 0; i < raw->num_lines; ++i) {
        free(raw->data[i]);
    }
    free(raw->data);

    free(raw);
}

/**
 * Counts filled (non-NULL) lines in `raw`.
 *
 * @param[in] raw RawInput to count lines of
 *
 * @return Number of filled lines in `raw`
 */
static int
RawInput_count_lines(const RawInput *raw)
{
    int ctr = 0;
    for (int i = 0; i < raw->num_lines; ++i) {
        if (raw->data[i] == NULL) {
            continue;
        }
        ++ctr;
    }
    return ctr;
}

/**
 * Comparison function for integers in the style of the C standard library.
 * Returns <0 if integer pointed to by `lhs` is less than that pointed to by
 * `rhs, >0 if greater than, 0 if equal.
 *
 * @param[in] lhs pointer to value of left hand side
 * @param[in] rhs pointer to value of right hand side
 *
 * @return <0 if `lhs` is less than `rhs, >0 if greater than, 0 if equal
 */
static int
_cmp_fnc_int(const void *lhs, const void *rhs)
{
    const int a = *(const int *) lhs;
    const int b = *(const int *) rhs;
    return (a > b) - (a < b);
}

/**
 * Checks if data in `input` is a valid game. If so, sets number of colors in
 * `input`.
 *
 * @param[in] input Input to check sanity of
 *
 * @return Error code
 */
int
Input_sanity_check(Input *input)
{
    const int num_tot = input->num_slots * input->num_tubes;
    int *cpy = malloc(num_tot * sizeof *cpy);
    cpy = memcpy(cpy, input->data, num_tot * sizeof *input->data);
    qsort(cpy, num_tot, sizeof *cpy, &_cmp_fnc_int);

    /* First, count empty slots, i.e., extra tubes */
    int idx = 0;
    for (; cpy[idx] == EMPTY_COLOR_INDEX; ++idx)
        ;
    if (idx == 0 || idx % input->num_slots != 0) {
        free(cpy);
        return TUBE_FAILURE;
    }
    const int num_extra = idx / input->num_slots;
    input->num_colors = input->num_tubes - num_extra;

    /* Then, count colors */
    int ctr = 1;
    for (; idx < num_tot - 1; ++idx) {
        if (cpy[idx] == cpy[idx + 1]) {
            ++ctr;
            continue;
        }
        if (ctr != input->num_slots) {
            free(cpy);
            return TUBE_FAILURE;
        }
        ctr = 1;
    }

    free(cpy);
    return TUBE_SUCCESS;
}

Input *
Input_read(const char *filename)
{
    RawInput *raw = RawInput_read(filename);
    if (raw == NULL) {
        return NULL;
    }

    Input *input = malloc(sizeof *input);

    input->num_slots = raw->num_elems;
    input->num_tubes = RawInput_count_lines(raw);

    const int num_tot = input->num_slots * input->num_tubes;
    input->data = malloc(num_tot * sizeof *input->data);

    int i_tube = 0;
    for (int i_line = 0; i_line < raw->num_lines; ++i_line) {
        if (raw->data[i_line] == NULL) {
            continue;
        }
        for (int i_elem = 0; i_elem < raw->num_elems; ++i_elem) {
            const int i_data = i_tube * raw->num_elems + i_elem;
            input->data[i_data] = raw->data[i_line][i_elem];
            if (input->data[i_data] < 0) {
                input->data[i_data] = EMPTY_COLOR_INDEX;
            }
        }
        ++i_tube;
    }

    RawInput_free(raw);

    if (Input_sanity_check(input) == TUBE_FAILURE) {
        ERROR("Input file '%s' failed sanity check!", filename);
    }

    return input;
}

void
Input_destroy(Input *input)
{
    if (input == NULL) {
        return;
    }

    free(input->data);

    free(input);
}
