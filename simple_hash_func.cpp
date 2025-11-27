#include "simple_hash_func.hpp"

static const uint64_t MWC64_ALPHA=0x00007F5454545437;
static const uint64_t FNV64_INITIAL_OFFSET=0xCBF29CE484222325;
static const uint64_t FNV64_PRIME=0x00000100000001B3;
static const uint64_t XORSHIFT64_ALPHA=0x2545F4914F6CDD1D;

uint64_t mwc_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result=(result & UINT32_MAX) * MWC64_ALPHA + (result >> 32);
	}
	return(result);
}

uint64_t fnv1a_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result^=data[i];
		result*=FNV64_PRIME;
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

uint64_t xorshiftstar_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t result=FNV64_INITIAL_OFFSET, i;
	for(i=0; i<len; i++)
	{
		result+=data[i];
		result^=result>>13;
		result^=result<<25;
		result^=result>>27;
		result*=XORSHIFT64_ALPHA;
	}
	return(result);
}
