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

#define C(x)	(char)((x) + '0')


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

static size_t
bcd32tostr(char *restrict buf, size_t bsz, uint_least32_t mant, int e, int s)
{
	char *restrict bp = buf;
	const char *const ep = buf + bsz;

	/* write the right-of-point side first */
	for (; e < 0 && bp < ep; e++, mant >>= 4U) {
		*bp++ = C(mant & 0b1111U);
	}
	/* write point now */
	if (bp > buf && bp < ep) {
		*bp++ = '.';
	}
	/* write trailing 0s for left-of-point side */
	for (; e > 0 && bp < ep; e--) {
		*bp++ = '0';
	}
	/* now write the rest of the mantissa */
	if (LIKELY(mant)) {
		for (; mant && bp < ep; mant >>= 4U) {
			*bp++ = C(mant & 0b1111U);
		}
	} else if (bp < ep) {
		/* put a leading 0 */
		*bp++ = '0';
	}
	if (s && bp < ep) {
		*bp++ = '-';
	}
	if (bp < ep) {
		*bp = '\0';
	}
	/* reverse the string */
	for (char *ip = buf, *jp = bp - 1; ip < jp; ip++, jp--) {
		char tmp = *ip;
		*ip = *jp;
		*jp = tmp;
	}
	return bp - buf;
}


int
bid32tostr(char *restrict buf, size_t UNUSED(bsz), _Decimal32 x)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	int e;
	int s;
	uint_least32_t m;

	/* get the exponent, sign and mantissa */
	e = quantexpbid32(x);
	m = mant_bid(x);
	s = m ? sign_bid(x) : 0/*no stinking signed naughts*/;

	/* reencode m as bcd */
	with (uint_least32_t bcdm = 0U) {
		for (size_t i = 0; i < 7U; i++, bcdm >>= 4U) {
			uint_least32_t digit;

			digit = m % 10U, m /= 10U;
			bcdm |= digit << 28U;
		}
		m = bcdm;
	}
	return (int)bcd32tostr(buf, bsz, m, e, s);
}

int
dpd32tostr(char *restrict buf, size_t UNUSED(bsz), _Decimal32 x)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	int e;
	int s;
	uint_least32_t m;

	/* get the exponent, sign and mantissa */
	e = quantexpdpd32(x);
	if (LIKELY((m = mant_dpd(x)))) {
		uint_least32_t l3;
		uint_least32_t h3;

		s = sign_dpd(x);
		/* get us a proper bcd version of M */
		l3 = unpack_declet(m & 0b1111111111U);
		m >>= 10U;
		h3 = unpack_declet(m & 0b1111111111U);
		m >>= 10U;
		m <<= 24U;
		m |= h3 << 12U;
		m |= l3 << 0U;
	} else {
		/* no stinking signed 0s and m is in bcd form already */
		s = 0;
	}
	return bcd32tostr(buf, bsz, m, e, s);
}

int
d32tostr(char *restrict buf, size_t bsz, _Decimal32 x)
{
#if defined HAVE_DFP754_BID_LITERALS
	return bid32tostr(buf, bsz, x);
#elif defined HAVE_DFP754_DPD_LITERALS
	return dpd32tostr(buf, bsz, x);
#endif	 /* HAVE_DFP754_*_LITERALS */
}

/* pr-d32.c ends here */
