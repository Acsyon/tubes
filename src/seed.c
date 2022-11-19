#include "seed.h"

#include <stdlib.h>
#include <time.h>

unsigned int
get_seed(void)
{
    srand(time(NULL));
    size_t hash1 = (size_t) rand();
    srand(clock());
    size_t hash2 = (size_t) rand();
    hash1 += 0x9e3779b9 + (hash2 << 6) + (hash2 >> 2);
    return (unsigned int) (hash2 ^ hash1);
}
