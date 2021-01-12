/*
 * udiv3.h
 *
 *  Created on: 03.01.2010
 *      Author: stefan
 */

#ifndef UDIV3_H_
#define UDIV3_H_

/*
 * Define the order of 32-bit words in 64-bit words.
 */
#ifdef _LONG_LONG_LTOH
#define	_QUAD_HIGHWORD 1
#define	_QUAD_LOWWORD 0
#else
#define	_QUAD_HIGHWORD 0
#define	_QUAD_LOWWORD 1
#endif

#define CHAR_BIT	8
#define	QUAD_BITS	(sizeof (u_longlong_t) * CHAR_BIT)
#define	LONG_BITS	(sizeof (long) * CHAR_BIT)
#define	HALF_BITS	(sizeof (long) * CHAR_BIT / 2)

/*
 * Depending on the desired operation, we view a `long long' in
 * one or more of the following formats.
 */
union uu {
	UINT64		uq;		/* as an unsigned quad */
	SINT64		q;		/* as a (signed) quad */

	SINT32		sl[2];		/* as two signed longs */
	UINT32		ul[2];		/* as two unsigned longs */
};

/*
 * Define high and low longwords.
 */
#define	H		_QUAD_HIGHWORD
#define	L		_QUAD_LOWWORD

/*
 * Extract high and low shortwords from longword, and move low shortword of
 * longword to upper half of long, i.e., produce the upper longword of
 * ((longlong_t)(x) << (number_of_bits_in_long/2)).
 * (`x' must actually be ulong_t.)
 *
 * These are used in the multiply code, to split a longword into upper
 * and lower halves, and to reassemble a product as a longlong_t, shifted left
 * (sizeof(long)*CHAR_BIT/2).
 */
#define	HHALF(x)	((x) >> HALF_BITS)
#define	LHALF(x)	((x) & ((1 << HALF_BITS) - 1))
#define	LHUP(x)		((x) << HALF_BITS)

#endif /* UDIV3_H_ */
