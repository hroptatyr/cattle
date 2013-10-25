/*** rn-d32.c -- _Decimal32 rounder
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
sign_bid(_Decimal32 x)
{
	uint32_t b = bits(x);
	return (int32_t)b >> 31U;
}

static inline __attribute__((pure, const, unused)) uint_least32_t
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

static _Decimal32
quantizebid32(_Decimal32 x, _Decimal32 r)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	int er;
	int ex;
	uint_least32_t m;

	/* get the exponent and mantissa */
	er = quantexpbid32(r);
	ex = quantexpbid32(x);

	m = mant_bid(x);

	/* truncate */
	for (; ex < er; ex++) {
		m = m / 10U + ((m % 10U) >= 5U);
	}
	/* expand (only if we don't exceed the range) */
	for (; m < 1000000U && ex > er; ex--) {
		m *= 10U;
	}

	/* assemble the d32 */
	with (uint32_t u = sign_bid(x) << 31U) {
		/* check if 24th bit of mantissa is set */
		if (UNLIKELY(m & (1U << 23U))) {
			u |= 0b11U << 29U;
			u |= (unsigned int)(ex + 101) << 21U;
			/* just use 21 bits of the mantissa */
			m &= 0x1fffffU;
		} else {
			u |= (unsigned int)(ex + 101) << 23U;
			/* use all 23 bits */
			m &= 0x7fffffU;
		}
		u |= m;
		x = bobs(u);
	}
	return x;
}

#if !defined HAVE_SCALBND32
static _Decimal32
scalbnbid32(_Decimal32 x, int n)
{
	/* just fiddle with the exponent of X then */
	with (uint32_t b = bits(x), u) {
		/* the idea is to xor the current expo with the new expo
		 * and shift the result to the right position and xor again */
		if (UNLIKELY((b & 0x60000000U) == 0x60000000U)) {
			/* 24th bit of mantissa is set, special expo */
			u = (b >> 21U) & 0xffU;
			u ^= u + n;
			u &= 0xffU;
			u <<= 21U;
		} else {
			u = (b >> 23U) & 0xffU;
			u ^= u + n;
			u &= 0xffU;
			u <<= 23U;
		}
		b ^= u;
		x = bobs(b);
	}
	return x;
}
#endif	/* !HAVE_SCALBND32 */


_Decimal32
quantized32(_Decimal32 x, _Decimal32 r)
{
	return quantizebid32(x, r);
}

#if !defined HAVE_SCALBND32
_Decimal32
scalbnd32(_Decimal32 x, int n)
{
	return scalbnbid32(x, n);
}
#endif	/* !HAVE_SCALBND32 */

/* rn-d32.c ends here */
