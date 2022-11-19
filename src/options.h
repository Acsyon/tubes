/** options.h
 *
 * Header for handling command line options for 'tubes'. Basically, a very crude
 * version of 'getopt'.
 */

#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <stdbool.h>

/**
 * Struct for command line options.
 */
typedef struct {
    char shortopt;
    const char *longopt;
    bool has_arg;
} ProgramOption;

/**
 * Cheap getopt ersatz. Checks whether `argv[*p_argidx]` corresponds to `opt`.
 * If so, returns 'true' and sets `p_optarg` to potential option argument. If
 * not, returns 'false'.
 *
 * @param[in] opt ProgramOption to check
 * @param[in,out] p_argidx pointer to argument index of argument to be checked
 * @param[in] argv argument vector of main
 * @param[out] p_optarg pointer to option argument
 *
 * @return Does `argv[*p_argidx]` correspond to `opt`?
 */
bool
ProgramOption_check(
  const ProgramOption *opt, int *p_argidx, char **argv, char **p_optarg
);

#endif /* OPTIONS_H_INCLUDED */
