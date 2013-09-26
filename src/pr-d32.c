/*** pr-d32.c -- _Decimal32 printer
 *
 * Copyright (C) 2013 Sebastian Freundt
 *
 * Author:  Sebastian Freundt <freundt@ga-group.nl>
 *
 * This file is part of cattle.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the author nor the names of any contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ***/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdint.h>
#if defined HAVE_DFP754_H
# include <dfp754.h>
#elif defined HAVE_DFP_STDLIB_H
# include <dfp/stdlib.h>
#endif	/* HAVE_DFP754_H */
#include "ctl-dfp754.h"
#include "nifty.h"

static inline __attribute__((pure, const)) int
expo_bid(_Decimal32 x)
{
	uint32_t b = bits(x);
	register int tmp;

	if (UNLIKELY((b & 0x60000000U) == 0x60000000U)) {
		/* exponent starts 2 bits to the left */
		tmp = (b >> 21U);
	} else {
		tmp = (b >> 23U);
	}
	return (tmp & 0xffU) - 101;
}

static inline __attribute__((pure, const)) int
expo_dpd(_Decimal32 x)
{
	uint32_t b = bits(x);
	register int tmp;

	b >>= 20U;
	if (UNLIKELY((b & 0b11000000000U) == 0b11000000000U)) {
		/* exponent starts 2 bits to the left */
		tmp = ((b & 0b00110000000U) >> 1U) | (b & 0b111111U);
	} else {
		tmp = ((b & 0b11000000000U) >> 3U) | (b & 0b111111U);
	}
	return tmp - 101;
}

static inline __attribute__((pure, const)) int
sign_bid(_Decimal32 x)
{
	uint32_t b = bits(x);
	return (int32_t)b >> 31U;
}

static inline __attribute__((pure, const)) int
sign_dpd(_Decimal32 x)
{
	uint32_t b = bits(x);
	return (int32_t)b >> 31U;
}

static inline __attribute__((pure, const)) uint_least32_t
mant_bid(_Decimal32 x)
{
	uint32_t b = bits(x);
	register uint_least32_t res;

	if (UNLIKELY((b & 0x60000000U) == 0x60000000U)) {
		/* mantissa is only 21 bits + 0b100 */
		res = 0b100U << 21U;
		res |= (b & 0x1fffffU);
	} else {
		/* mantissa is full */
		res = b & 0x7fffffU;
	}
	return res;
}

static inline __attribute__((pure, const)) uint_least32_t
mant_dpd(_Decimal32 x)
{
	uint32_t b = bits(x);
	register uint_least32_t res;

	/* at least the last 20 bits aye */
	res = b & 0xfffffU;
	b >>= 26U;
	if (UNLIKELY((b & 0b11000U) == 0b11000U)) {
		/* exponent is two more bits, then the high bit */
		res |= (b & 0b00001U | 0b1000U) << 20U;
	} else {
		res |= (b & 0b00111U) << 20U;
	}
	return res;
}

static unsigned int
unpack_declet(unsigned int x)
{
/* go from dpd to bcd, here's the dpd box again: 
 * abc def 0ghi
 * abc def 100i
 * abc ghf 101i
 * ghc def 110i
 * abc 10f 111i
 * dec 01f 111i
 * ghc 00f 111i
 * ??c 11f 111i
 */
	unsigned int res = 0U;

	/* check for the easiest case first */
	if (!(x & 0b1000U)) {
		goto trivial;
	} else {
		switch ((x & 0b1110U) >> 1U) {
		case 0b100U:
		trivial:
			res |= x & 0b1111U;
			res |= (x & 0b1110000U);
			res |= (x & 0b1110000000U) << 1U;
			break;
		case 0b101U:
			/* ghf -> 100f ghi */
			res |= (x & 0b1110000000U) << 1U;
			res |= (0b1000U << 4U) | (x & 0b0010000U);
			res |= ((x & 0b1100000U) >> 4U) | (x & 0b1U);
			break;
		case 0b110U:
			/* ghc -> 100c ghi */
			res |= (0b1000U << 8U) | ((x & 0b0010000000U) << 1U);
			res |= (x & 0b1110000U);
			res |= ((x & 0b1100000000U) >> 7U) | (x & 0b1U);
			break;
		case 0b111U:
			/* grrr */
			switch ((x & 0b1100000U) >> 5U) {
			case 0b10U:
				res |= (0b1110000000U << 1U);
				res |= (0b1000U << 4U) | (x & 0b0000010000U);
				res |= (0b1000U << 0U) | (x & 0b0000000001U);
				break;
			case 0b01U:
				res |= (0b1000U << 8U) |
					((x & 0b0010000000U) << 1U);
				res |= ((x & 0b1100000000U) >> 3U) |
					(x & 0b0000010000U);
				res |= (0b1000U << 0U) | (x & 0b0000000001U);
				break;
			case 0b00U:
				res |= (0b1000U << 8U) |
					((x & 0b0010000000U) << 1U);
				res |= (0b1000U << 4U) | (x & 0b0000010000U);
				res |= ((x & 0b1100000000U) >> 7U) | (x & 0b1U);
				break;
			case 0b11U:
				res |= (0b1000U << 8U) |
					((x & 0b0010000000U) << 1U);
				res |= (0b1000U << 4U) | (x & 0b0000010000U);
				res |= (0b1000U << 0U) | (x & 0b0000000001U);
				break;
			}
			break;
		}
	}
	return res;
}


int
bid32tostr(char *restrict buf, size_t UNUSED(bsz), _Decimal32 x)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	static unsigned int d[] = {
		1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U,
	};
	int e;
	uint_least32_t m;
	uint_least32_t pre;
	/* point pointer */
	char *restrict bp = buf;
	char *restrict pp;

	if (UNLIKELY((e = -expo_bid(x)) < 0 || e >= 8)) {
		return 0;
	}
	if (!(m = mant_bid(x))) {
		/* we don't want no stinking signed 0s so branch here and
		 * skip the next one */
		;
	} else if (sign_bid(x)) {
		*bp++ = '-';
	}
	/* decompose into left-of-point and right-of-point halves */
	pre = m / d[e], m %= d[e];
	/* default point point */
	pp = bp + 1U;
	/* find the right spot though */
	for (size_t i = 1U; i < countof(d) && pre >= d[i]; i++, pp++);

	/* write digits right of the point */
	for (size_t i = e; i > 0U; i--) {
		pp[i] = (char)((m % 10U) + '0');
		m /= 10U;
	}
	/* write the left portion */
	for (size_t i = pp - bp; i > 0U; i--) {
		bp[i - 1U] = (char)((pre % 10U) + '0');
		pre /= 10U;
	}
	/* write the point, and move pp */
	if (LIKELY(e > 0)) {
		*pp = '.';
		pp += e + 1U;
	}
	*pp = '\0';
	return pp - buf;
}

int
dpd32tostr(char *restrict buf, size_t UNUSED(bsz), _Decimal32 x)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
#define C(x)	(char)((x) + '0')
	int e;
	int nd = 1;
	uint_least32_t m;
	uint_least32_t suf;
	/* point pointer */
	char *restrict bp = buf;

	if (UNLIKELY((e = -expo_dpd(x)) < 0 || e >= 8)) {
		return 0;
	}
	if (!(m = mant_dpd(x))) {
		/* we don't want no stinking signed 0s so branch here and
		 * skip the next one */
		;
	} else {
		uint_least32_t l3;
		uint_least32_t h3;

		if (sign_dpd(x)) {
			*bp++ = '-';
		}
		/* get us a proper bcd version of M */
		l3 = unpack_declet(m & 0b1111111111U);
		m >>= 10U;
		h3 = unpack_declet(m & 0b1111111111U);
		m >>= 10U;
		m <<= 24U;
		m |= h3 << 12U;
		m |= l3 << 0U;
	}
	/* get the number of digits */
	for (register uint_least32_t c = m >> 4U; c; c >>= 4U, nd++);
	suf = m & ((1U << (4U * e)) - 1U);
	m >>= 4U * e;
	if (nd < e) {
		nd = e;
	}
	/* just print the numbers, nd - e digits left, e digits right */
	switch (nd - e) {
	default:
		return 0;
	case 7U:
		bp[6] = C(m & 0b1111U), m >>= 4U;
	case 6U:
		bp[5] = C(m & 0b1111U), m >>= 4U;
	case 5U:
		bp[4] = C(m & 0b1111U), m >>= 4U;
	case 4U:
		bp[3] = C(m & 0b1111U), m >>= 4U;
	case 3U:
		bp[2] = C(m & 0b1111U), m >>= 4U;
	case 2U:
		bp[1] = C(m & 0b1111U), m >>= 4U;
	case 1U:
		bp[0] = C(m & 0b1111U);
		bp += nd - e;
		break;
	case 0U:
		*bp++ = '0';
		break;
	}
	if (e) {
		*bp++ = '.';
		switch (e) {
		default:
			return 0;
		case 7U:
			bp[6] = C(suf & 0b1111U), suf >>= 4U;
		case 6U:
			bp[5] = C(suf & 0b1111U), suf >>= 4U;
		case 5U:
			bp[4] = C(suf & 0b1111U), suf >>= 4U;
		case 4U:
			bp[3] = C(suf & 0b1111U), suf >>= 4U;
		case 3U:
			bp[2] = C(suf & 0b1111U), suf >>= 4U;
		case 2U:
			bp[1] = C(suf & 0b1111U), suf >>= 4U;
		case 1U:
			bp[0] = C(suf & 0b1111U);
			bp += e;
			break;
		}
	}
	/* finalise and that's it */
	*bp = '\0';
	return bp - buf;
#undef C
}

int
d32tostr(char *restrict buf, size_t bsz, _Decimal32 x)
{
	return bid32tostr(buf, bsz, x);
}

/* pr-d32.c ends here */
