#include "simple_hash_func.hpp"

static const uint32_t MMS32_ALPHA=0x7F545415;
static const uint64_t MMS64_ALPHA=0x000007F22254544B;

static const uint32_t FNV32_INITIAL_OFFSET=0x811C9DC5;
static const uint64_t FNV64_INITIAL_OFFSET=0xCBF29CE484222325;

static const uint32_t FNV32_PRIME=0x01000193;
static const uint64_t FNV64_PRIME=0x00000100000001B3;

uint32_t mwc_hash_32(const uint8_t *data, uint32_t len)
{
	uint32_t result=FNV32_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result=(result & UINT16_MAX) * MMS32_ALPHA + (result >> 16);
	}
	return(result);
}

uint64_t mwc_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result=(result & UINT32_MAX) * MMS64_ALPHA + (result >> 32);
	}
	return(result);
}

uint32_t fnv1a_hash_32(const uint8_t *inbuf, uint32_t inbuf_len)
{
	uint32_t result=FNV32_INITIAL_OFFSET, i;
	for(i=0; i<inbuf_len; i++)
	{
		result^=inbuf[i];
		result*=FNV32_PRIME;
	}
	return(result);
}

uint64_t fnv1a_hash_64(const uint8_t *inbuf, uint64_t inbuf_len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<inbuf_len; i++)
	{
		result^=inbuf[i];
		result*=FNV64_PRIME;
	}
	return(result);
}

uint32_t xorshift_hash_32(const uint8_t *data, uint32_t len)
{
	uint32_t result=FNV32_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result^=result<<13;
		result^=result>>17;
		result^=result<<5;
	}
	return(result);
}

uint64_t xorshift_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result^=result<<13;
		result^=result>>7;
		result^=result<<17;
	}
	return(result);
}
