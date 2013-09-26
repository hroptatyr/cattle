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

#if !defined HAVE_DFP754_H && !defined HAVE_DFP_STDLIB_H
_Decimal32
strtobid32(const char *src, char **on)
{
	return strtobid32(src, on);
}
#endif	/* !HAVE_DFP754_H && !HAVE_DFP_STDLIB_H */

/* rd-d32.c ends here */
