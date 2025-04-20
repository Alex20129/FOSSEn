#include "simple_hash_func.hpp"

static const uint32_t MMSHASH_ALPHA_32=0xBC243A87;
static const uint32_t MMSHASH_START_VALUE_32=0xDEADBEEF;

static const uint64_t MMSHASH_ALPHA_64=0x258F6A35BC243A87;
static const uint64_t MMSHASH_START_VALUE_64=0xDEADBEEFDEADBEEF;

uint32_t mms_hash_32(const uint8_t *data, uint32_t len)
{
	uint32_t mms_result=MMSHASH_START_VALUE_32, i;
	for (i=0; i < len; i++)
	{
		mms_result+=data[i];
		mms_result=(mms_result & UINT16_MASK) * MMSHASH_ALPHA_32 + (mms_result >> 16);
	}
	return (mms_result);
}

uint64_t mms_hash_64(const uint8_t *data, uint64_t len)
{
	uint64_t mms_result=MMSHASH_START_VALUE_64, i;
	for (i=0; i < len; i++)
	{
		mms_result+=data[i];
		mms_result=(mms_result & UINT32_MASK) * MMSHASH_ALPHA_64 + (mms_result >> 32);
	}
	return (mms_result);
}

static const uint32_t FNV32_OFFSET =0x811C9DC5;
static const uint32_t FNV32_PRIME  =0x01000193;

static const uint64_t FNV64_OFFSET =0xCBF29CE484222325;
static const uint64_t FNV64_PRIME  =0x00000100000001B3;

uint32_t fnv1a_hash_32(const uint8_t *inbuf, uint32_t inbuf_len)
{
	uint32_t fnv_result=FNV32_OFFSET, i;
	for (i=0; i < inbuf_len; i++)
	{
		fnv_result^=inbuf[i];
		fnv_result*=FNV32_PRIME;
	}
	return (fnv_result);
}

uint64_t fnv1a_hash_64(const uint8_t *inbuf, uint64_t inbuf_len)
{
	uint64_t fnv_result=FNV64_OFFSET, i;
	for (i=0; i < inbuf_len; i++)
	{
		fnv_result^=inbuf[i];
		fnv_result*=FNV64_PRIME;
	}
	return (fnv_result);
}
