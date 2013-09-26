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
#include "ctl-dfp754.h"
#include "nifty.h"

typedef struct _d32_bid_s _d32_bid_t;

struct _d32_bid_s {
#if defined WORDS_BIGENDIAN
	int s:1;
	unsigned int e:8;
	unsigned int m:23;
#else  /* !WORDS_BIGENDIAN */
	unsigned int m:23;
	unsigned int e:8;
	int s:1;
#endif	/* WORDS_BIGENDIAN */
};

static inline __attribute__((pure, const)) uint32_t
bits(_Decimal32 x)
{
	return (union {_Decimal32 x; uint32_t u;}){x}.u;
}

static inline __attribute__((pure, const)) int
expo(_Decimal32 x)
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
sign(_Decimal32 x)
{
	uint32_t b = bits(x);
	return (int32_t)b >> 31;
}

static inline __attribute__((pure, const)) uint_least32_t
mant(_Decimal32 x)
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


int
d32tostr(char *restrict buf, size_t UNUSED(bsz), _Decimal32 x)
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

	if (UNLIKELY((e = -expo(x)) < 0 || e >= 8)) {
		return 0;
	}
	if (!(m = mant(x))) {
		/* we don't want no stinking signed 0s so branch here and
		 * skip the next one */
		;
	} else if (sign(x)) {
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
	for (size_t i = pp - buf; i > 0U; i--) {
		buf[i - 1U] = (char)((pre % 10U) + '0');
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

/* pr-d32.c ends here */
