#ifndef SIMPLE_HASH_FUNC_H
#define SIMPLE_HASH_FUNC_H

#include <stdint.h>

uint32_t mwc_hash_32(const uint8_t *data, uint32_t len);
uint64_t mwc_hash_64(const uint8_t *data, uint64_t len);

uint32_t fnv1a_hash_32(const uint8_t *inbuf, uint32_t inbuf_len);
uint64_t fnv1a_hash_64(const uint8_t *inbuf, uint64_t inbuf_len);

uint32_t xorshift_hash_32(const uint8_t *data, uint32_t len);
uint64_t xorshift_hash_64(const uint8_t *data, uint64_t len);

#endif // SIMPLE_HASH_FUNC_H
