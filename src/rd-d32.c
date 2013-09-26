/*** rd-d32.c -- _Decimal32 reader
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

static unsigned int
pack_declet(unsigned int x)
{
/* consider X to be 12bit BCD of 3 digits, return the declet
 * X == BCD(d2, d1, d0) */
	unsigned int res;
	unsigned int d0;
	unsigned int d1;
	unsigned int d2;
	unsigned int c = 0U;

	if (UNLIKELY((d2 = (x >> 8U) & 0xfU) >= 8U)) {
		/* d2 is large, 8 or 9, 0b1000 or 0b1001, store only low bit */
		d2 &= 0x1U;
		c |= 0b100U;
	}

	if (UNLIKELY((d1 = (x >> 4U) & 0xfU) >= 8U)) {
		/* d1 is large, 8 or 9, 0b1000 or 0b1001, store only low bit */
		d1 &= 0x1U;
		c |= 0b010U;
	}

	if (UNLIKELY((d0 = (x >> 0U) & 0xfU) >= 8U)) {
		/* d0 is large, 8 or 9, 0b1000 or 0b1001, store only low bit */
		d0 &= 0x1U;
		c |= 0b001U;
	}

	/* generally the low bits in the large case coincide */
	res = (d2 << 7U) | (d1 << 4U) | d0;

	switch (c) {
	case 0b000U:
		break;
	case 0b001U:
		/* we need d2d1(100)d0 */
		res |= 0b100U << 1U;
		break;
	case 0b010U:
		/* this is d2(gh)d1(101)(i) for d0 = ghi */
		res |= (0b100U << 1U) | ((d0 & 0b110U) << 4U);
		break;
	case 0b011U:
		/* this goes to d2(10)d1(111)d0 */
		res |= (0b100U << 4U) | (0b111U << 1U);
		break;
	case 0b100U:
		/* this goes to (gh)d2d1(110)(i) for d0 = ghi */
		res |= ((d0 & 0b110U) << 7U) | (0b110U << 1U);
		break;
	case 0b101U:
		/* this will be (de)d2(01)f(111)d0 for d1 = def */
		res |= ((d1 & 0b110U) << 7U) | (0b010U << 4U) | (0b111U << 1U);
		break;
	case 0b110U:
		/* goes to (gh)d2(00)d1(111)(i) for d0 = ghi */
		res |= ((d0 & 0b110U) << 7U) | (0b111U << 1U);
		break;
	case 0b111U:
		/* goes to (xx)d2(11)d1(111)d0 */
		res |= (0b110U << 04U) | (0b111U << 1U);
		break;
	default:
		res = 0U;
		break;
	}
	return res;
}


_Decimal32
strtobid32(const char *src, char **on)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	uint_least32_t mant = 0U;
	int sign = 0U;
	int expo = 0U;
	const char *sp = src;
	_Decimal32 res;

	if (UNLIKELY(*sp == '-')) {
		sign = 1;
		sp++;
	}
	for (; *sp >= '0' && *sp <= '9'; sp++) {
		mant *= 10U;
		mant += *sp - '0';
	}
	switch (*sp) {
	case '.':
		expo = 0;
		for (sp++; *sp >= '0' && *sp <= '9'; sp++, expo--) {
			mant *= 10U;
			mant += *sp - '0';
		}
		break;
	default:
		break;
	}

	if (LIKELY(on != NULL)) {
		*on = deconst(sp);
	}

	/* assemble the d32 */
	with (uint32_t u) {
		u = sign << 31U;
		/* check if 24th bit of mantissa is set */
		if (UNLIKELY(mant & (1U << 23U))) {
			u |= 0b11U << 29U;
			u |= (unsigned int)(expo + 101) << 21U;
			/* just use 21 bits of the mantissa */
			mant &= 0x1fffffU;
		} else {
			u |= (unsigned int)(expo + 101) << 23U;
			/* use all 23 bits */
			mant &= 0x7fffffU;
		}
		u |= mant;
		res = bobs(u);
	}
	return res;
}

_Decimal32
strtodpd32(const char *src, char **on)
{
/* d32s look like s??eeeeee mm..23..mm
 * and the decimal is (-1 * s) * m * 10^(e - 101),
 * this implementation is very minimal serving only the cattle use cases */
	uint_least32_t mant = 0U;
	int sign = 0U;
	int expo = 0U;
	const char *sp = src;
	_Decimal32 res;

	if (UNLIKELY(*sp == '-')) {
		sign = 1;
		sp++;
	}
	for (; *sp >= '0' && *sp <= '9'; sp++) {
		mant <<= 4U;
		mant |= (*sp - '0');
	}
	switch (*sp) {
	case '.':
		expo = 0;
		for (sp++; *sp >= '0' && *sp <= '9'; sp++, expo--) {
			mant <<= 4U;
			mant |= (*sp - '0');
		}
		break;
	default:
		break;
	}

	if (LIKELY(on != NULL)) {
		*on = deconst(sp);
	}

	/* pack the mantissa */
	{
		/* lower 3 digits */
		uint_least32_t l3 = pack_declet(mant & 0xfffU);
		uint_least32_t u3 = pack_declet((mant & 0xfff000U) >> 12U);
		uint_least32_t u7 = (mant >> 24U) & 0xfU;
		unsigned int rexp = expo + 101;
		uint32_t u;

		/* assemble the d32 */
		u = sign << 31U;
		/* check if bits 24-27 (d7) is small or large */
		if (UNLIKELY(u7 >= 8U)) {
			u |= 0b11U << 29U;
			u |= (rexp & 0b11000000U) << 21U;
		} else {
			u |= (rexp & 0b11000000U) << 23U;
		}
		/* the bottom rexp bits */
		u |= (rexp & 0b00111111U) << 20U;
		/* now comes u7 */
		u |= (u7 & 0x7U) << 26U;
		u |= u3 << 10U;
		u |= l3 << 0U;
		res = bobs(u);
	}
	return res;
}

#if !defined HAVE_DFP754_H && !defined HAVE_DFP_STDLIB_H
_Decimal32
strtod32(const char *src, char **on)
{
	return strtobid32(src, on);
}
#endif	/* !HAVE_DFP754_H && !HAVE_DFP_STDLIB_H */

/* rd-d32.c ends here */
