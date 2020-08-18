
#ifndef _HASH_H_
#define _HASH_H_

typedef unsigned int _u32;
typedef unsigned char _u8;
typedef signed char _s8;
typedef unsigned short _u16;


#define __hash_mix(a, b, c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

#define HASH_GOLDEN_RATIO	0x9e3779b9

static inline _u32 hash_ps8(const void *key, _u32 length, _u32 initval)
{
	_u32 a, b, c, len;
	const _u8 *k = key;

	len = length;
	a = b = HASH_GOLDEN_RATIO;
	c = initval;

	while (len >= 12) {
		a += (k[0] +((_u32)k[1]<<8) +((_u32)k[2]<<16) +((_u32)k[3]<<24));
		b += (k[4] +((_u32)k[5]<<8) +((_u32)k[6]<<16) +((_u32)k[7]<<24));
		c += (k[8] +((_u32)k[9]<<8) +((_u32)k[10]<<16)+((_u32)k[11]<<24));

		__hash_mix(a,b,c);

		k += 12;
		len -= 12;
	}

	c += length;
	switch (len) {
	case 11: c += ((_u32)k[10]<<24);
	case 10: c += ((_u32)k[9]<<16);
	case 9 : c += ((_u32)k[8]<<8);
	case 8 : b += ((_u32)k[7]<<24);
	case 7 : b += ((_u32)k[6]<<16);
	case 6 : b += ((_u32)k[5]<<8);
	case 5 : b += k[4];
	case 4 : a += ((_u32)k[3]<<24);
	case 3 : a += ((_u32)k[2]<<16);
	case 2 : a += ((_u32)k[1]<<8);
	case 1 : a += k[0];
	};

	__hash_mix(a,b,c);

	return c;
}

static inline _u32 hash_pu32(const _u32 *k, _u32 length, _u32 initval)
{
	_u32 a, b, c, len;

	a = b = HASH_GOLDEN_RATIO;
	c = initval;
	len = length;

	while (len >= 3) {
		a += k[0];
		b += k[1];
		c += k[2];
		__hash_mix(a, b, c);
		k += 3; len -= 3;
	}

	c += length * 4;

	switch (len) {
	case 2 : b += k[1];
	case 1 : a += k[0];
	};

	__hash_mix(a,b,c);

	return c;
}

static inline _u32 hash_3xu32(_u32 a, _u32 b, _u32 c, _u32 initval)
{
	a += HASH_GOLDEN_RATIO;
	b += HASH_GOLDEN_RATIO;
	c += initval;

	__hash_mix(a, b, c);

	return c;
}

static inline _u32 hash_2xu32(_u32 a, _u32 b, _u32 initval)
{
	return hash_3xu32(a, b, 0, initval);
}

static inline _u32 hash_1xu32(_u32 a, _u32 initval)
{
	return hash_3xu32(a, 0, 0, initval);
}

static inline _u32 hash_dek(const _s8 *str)
{
	_u32 hash = 1315423911;
	_s8 ch;
	
	while((ch = *str++))
	{
		hash = ((hash << 5) ^ (hash >> 27)) ^ ch;
	}

	return hash;
}

static inline _u32 hash_fnv(const _s8 *str)
{
	_u32 hash = 2166136261UL;
	_s8 ch;
	while((ch = *str++))
	{
		hash *= 16777619;
		hash ^= ch;
	}

	return hash;
}

static inline _u32 hash_sdbm(const _s8 *str)
{
	_u32 hash = 0;
	while(*str)
	{
		hash = (*str++) + (hash << 6) + (hash << 16) - hash;
	}

	return hash;
}

#endif

