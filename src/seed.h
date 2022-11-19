/** seed.h
 *
 * Header for random seed creation of 'tubes'.
 */

#ifndef SEED_H_INCLUDED
#define SEED_H_INCLUDED

/**
 * Creates "hashes" (i.e., size_t) from time() and clock() and uses
 * boost::hash_combine() to create random number. I didn't check, however, if
 * this is actually a good choice. Could be total horseshit...
 *
 * If we were on C11, we could use timespec.tv_nsec. But I hate myself and so we
 * aren't...
 *
 * @return "random" seed
 */
unsigned int
get_seed(void);

#endif /* SEED_H_INCLUDED */
