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

#define C(x)	(char)((x) + '0')


static inline __attribute__((pure, const, unused)) int
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

#if !defined HAVE_QUANTIZED32
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
	er = LIKELY(bits(r)) ? expo_bid(r) : 0;
	ex = LIKELY(bits(x)) ? expo_bid(x) : 0;

	m = mant_bid(x);

	/* truncate */
	for (; ex < er; ex++) {
		m = m / 10U + ((m % 10U) >= 5U);
	}
	/* expand */
	for (; ex > er; ex--) {
		m *= 10U;
	}

	/* assemble the d32 */
	with (uint32_t u = sign_bid(x) << 31U) {
		/* check if 24th bit of mantissa is set */
		if (UNLIKELY(m & (1U << 23U))) {
			u |= 0b11U << 29U;
			u |= (unsigned int)(er + 101) << 21U;
			/* just use 21 bits of the mantissa */
			m &= 0x1fffffU;
		} else {
			u |= (unsigned int)(er + 101) << 23U;
			/* use all 23 bits */
			m &= 0x7fffffU;
		}
		u |= m;
		x = bobs(u);
	}
	return x;
}
#endif	/* !HAVE_QUANTIZED32 */


#if !defined HAVE_QUANTIZED32
_Decimal32
quantized32(_Decimal32 x, _Decimal32 r)
{
	return quantizebid32(x, r);
}
#endif	/* !HAVE_QUANTIZED32 */

/* rn-d32.c ends here */
