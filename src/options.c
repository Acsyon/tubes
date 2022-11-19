#include "options.h"

#include <stddef.h>
#include <string.h>

bool
ProgramOption_check(
  const ProgramOption *opt, int *p_argidx, char **argv, char **p_optarg
)
{
    *p_optarg = NULL;
    /* If it doesn't start with '-', it isn't an option */
    if (argv[*p_argidx][0] != '-') {
        return false;
    }
    /* First, check for short option */
    if (argv[*p_argidx][1] != '-') {
        /* This is not the option you're looking for */
        if (argv[*p_argidx][1] != opt->shortopt) {
            return false;
        }
        /* If option has no argument, we're done */
        if (opt->has_arg == false) {
            return true;
        }
        /* Option is either next element of argv or appended directly to
         * argument */
        if (argv[*p_argidx][2] == '\0') {
            *p_argidx += 1;
            *p_optarg = &argv[*p_argidx][0];
        } else {
            *p_optarg = &argv[*p_argidx][2];
        }
        return true;
    }
    /* Now check long option */
    const size_t optlen = strlen(opt->longopt);
    if (strncmp(&argv[*p_argidx][2], opt->longopt, optlen) != 0) {
        return false;
    }
    /* If option has no argument, we're done again */
    if (opt->has_arg == false) {
        return true;
    }
    /* Option is either next element of argv or appended directly to argument
     * with '=' */
    if (argv[*p_argidx][2 + optlen] == '\0') {
        *p_argidx += 1;
        *p_optarg = &argv[*p_argidx][0];
    } else if (argv[*p_argidx][2 + optlen] == '=') {
        *p_optarg = &argv[*p_argidx][3 + optlen];
    } else {
        return false;
    }

    return true;
}