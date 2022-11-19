#include <string.h>

#include "gameinfo.h"
#include "options.h"
#include "seed.h"
#include "util.h"

#define MAXIMUM_INPUT_FILENAME_LENGTH 256

#define DEFAULT_NUMBER_OF_COLORS 5
#define DEFAULT_NUMBER_OF_EXTRA_TUBES 2
#define DEFAULT_NUMBER_OF_SLOTS 4

/**
 * Enumerator for possible options.
 */
enum {
    OPT_h = 0,
    OPT_c,
    OPT_e,
    OPT_l,
    OPT_s,
    OPT_f,
    OPT_S,
    OPT_N,
};

/**
 * Possible options.
 */
static const ProgramOption OPTIONS[] = {
  [OPT_h] = {'h', "help", false},  [OPT_c] = {'c', "colors", true},
  [OPT_e] = {'e', "extra", true},  [OPT_l] = {'l', "slots", true},
  [OPT_s] = {'s', "seed", true},   [OPT_f] = {'f', "file", true},
  [OPT_S] = {'S', "solve", false}, [OPT_N] = {'N', "noplay", false},
};

/**
 * Usage string.
 */
static const char *const usage
  = "Usage: tubes [OPTION]...\n"
    "Generic \"colour sorting game\" (with solver).\n"
    "\n"
    "Options:\n"
    "  -h, --help    Show this help message and quit.\n"
    "  -c, --colors  Number of colors (default = 5)\n"
    "  -e, --extra   Number of extra tubes (default = 2)\n"
    "  -l, --slots   Number of slots per tube (default = 4)\n"
    "  -s, --seed    Random seed for game (default = random)\n"
    "  -f, --file    Read game from file instead of generating it from seed\n"
    "  -S, --solve   Print solution to file?\n"
    "  -N, --noplay  Do not actually play game?\n";

/**
 * Quick-and-dirty implementation of 'strnlen' to ensure it's available.
 *
 * @param[in] str pointer to the null-terminated byte string to be examined
 * @param[in] maxlen maximum number of characters to examine
 *
 * @return The length of the null-terminated byte string `str` on success, zero
 * if `str` is a null pointer, `maxlen` if the null character was not found
 */
static size_t
_strnlen(const char *str, size_t maxlen)
{
    if (str == NULL) {
        return 0;
    }
    size_t len;
    for (len = 0; len < maxlen; ++len) {
        if (str[len] == '\0') {
            break;
        }
    }
    return len;
}

/**
 * Quick-and-dirty implementation of 'strndup' to ensure it's available.
 *
 * @param[in] str pointer to the null-terminated byte string to duplicate
 * @param[in] maxlen maximum number of bytes to copy from `str`
 *
 * @return A pointer to the newly allocated string, or a null pointer if an
 * error occurred
 */
static char *
_strndup(const char *str, size_t maxlen)
{
    const size_t len = _strnlen(str, maxlen);
    if (len == 0) {
        return NULL;
    }
    char *dup = malloc(len + 1);
    dup = strncpy(dup, str, len);
    dup[len] = '\0';
    return dup;
}

int
main(int argc, char **argv)
{
    int num_colors = DEFAULT_NUMBER_OF_COLORS;
    int num_extra = DEFAULT_NUMBER_OF_EXTRA_TUBES;
    int num_slots = DEFAULT_NUMBER_OF_SLOTS;
    int seed = get_seed();
    char *filename = NULL;
    bool do_solve = false;
    bool do_noplay = false;

    char *optarg;
    for (int i = 1; i < argc; ++i) {
        if (ProgramOption_check(&OPTIONS[OPT_h], &i, argv, &optarg) == true) {
            printf("%s", usage);
            exit(EXIT_SUCCESS);
        }
        if (ProgramOption_check(&OPTIONS[OPT_c], &i, argv, &optarg) == true) {
            num_colors = atoi(optarg);
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_e], &i, argv, &optarg) == true) {
            num_extra = atoi(optarg);
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_l], &i, argv, &optarg) == true) {
            num_slots = atoi(optarg);
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_s], &i, argv, &optarg) == true) {
            seed = atol(optarg);
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_f], &i, argv, &optarg) == true) {
            filename = _strndup(optarg, MAXIMUM_INPUT_FILENAME_LENGTH);
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_S], &i, argv, &optarg) == true) {
            do_solve = true;
            continue;
        }
        if (ProgramOption_check(&OPTIONS[OPT_N], &i, argv, &optarg) == true) {
            do_noplay = true;
            continue;
        }
        ERROR("Unknown argument: '%s'\n\n%s", argv[i], usage);
    }

    if (num_colors < 1) {
        ERROR("Invalid number of colors: %i", num_colors);
    }
    if (num_extra < 1) {
        ERROR("Invalid number of extra tubes: %i", num_extra);
    }
    if (num_slots < 1) {
        ERROR("Invalid number of slots per tube: %i", num_slots);
    }

    GameInfo *info = NULL;
    if (filename == NULL) {
        info
          = GameInfo_create_from_seed(num_colors, num_extra, num_slots, seed);
    } else {
        info = GameInfo_create_from_file(filename);
    }
    if (do_solve == true) {
        GameInfo_solve(info);
    }
    if (do_noplay == false) {
        GameInfo_play(info);
    }
    GameInfo_destroy(info);

    free(filename);

    return EXIT_SUCCESS;
}
