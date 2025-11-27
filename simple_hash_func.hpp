#ifndef SIMPLE_HASH_FUNC_H
#define SIMPLE_HASH_FUNC_H

#include <stdint.h>

uint64_t mwc_hash_64(const uint8_t *data, uint64_t len);
uint64_t fnv1a_hash_64(const uint8_t *data, uint64_t len);
uint64_t xorshift_hash_64(const uint8_t *data, uint64_t len);
uint64_t xorshiftstar_hash_64(const uint8_t *data, uint64_t len);

#endif // SIMPLE_HASH_FUNC_H
