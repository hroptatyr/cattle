/*** ctl-dfp754.h -- _Decimal32 goodness
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
#if !defined INCLUDED_ctl_dfp754_h_
#define INCLUDED_ctl_dfp754_h_

#include <stdlib.h>
#include <stdint.h>

#define NAND32_U		(0x7c000000U)

extern int d32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtod32(const char*, char**);

extern int bid32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtobid32(const char*, char**);

extern int dpd32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtodpd32(const char*, char**);

/**
 * Round X to the quantum of R. */
extern _Decimal32 quantized32(_Decimal32 x, _Decimal32 r);

/**
 * Return X*10^N. */
extern _Decimal32 scalbnd32(_Decimal32 x, int n);


inline __attribute__((pure, const)) uint32_t bits(_Decimal32 x);
inline __attribute__((pure, const)) _Decimal32 bobs(uint32_t u);
inline __attribute__((pure, const)) int quantexpbid32(_Decimal32 x);
inline __attribute__((pure, const)) int quantexpdpd32(_Decimal32 x);
inline __attribute__((pure, const)) int quantexpd32(_Decimal32 x);
inline __attribute__((pure, const)) _Decimal32 nand32(char *__tagp);
#define isnand32		__builtin_isnand32

inline __attribute__((pure, const)) uint32_t
bits(_Decimal32 x)
{
	return (union {_Decimal32 x; uint32_t u;}){x}.u;
}

inline __attribute__((pure, const)) _Decimal32
bobs(uint32_t u)
{
	return (union {uint32_t u; _Decimal32 x;}){u}.x;
}

inline __attribute__((pure, const)) int
quantexpbid32(_Decimal32 x)
{
	register uint32_t b = bits(x);
	register int tmp;

	if (b == 0U) {
		return 0;
	} else if ((b & 0x60000000U) != 0x60000000U) {
		tmp = (b >> 23U);
	} else {
		/* exponent starts 2 bits to the left */
		tmp = (b >> 21U);
	}
	return (tmp & 0xffU) - 101;
}

inline __attribute__((pure, const)) int
quantexpdpd32(_Decimal32 x)
{
	register uint32_t b = bits(x);
	register int tmp;

	b >>= 20U;
	if (b == 0U) {
		return 0;
	} else if ((b & 0b11000000000U) != 0b11000000000U) {
		tmp = ((b & 0b11000000000U) >> 3U) | (b & 0b111111U);
	} else {
		/* exponent starts 2 bits to the left */
		tmp = ((b & 0b00110000000U) >> 1U) | (b & 0b111111U);
	}
	return tmp - 101;
}

inline __attribute__((pure, const)) int
quantexpd32(_Decimal32 x)
{
#if defined HAVE_DFP754_BID_LITERALS
	return quantexpbid32(x);
#elif defined HAVE_DFP754_DPD_LITERALS
	return quantexpdpd32(x);
#endif	/* HAVE_DFP754_*_LITERALS */
}

inline __attribute__((pure, const)) _Decimal32
nand32(char *__tagp __attribute__((unused)))
{
	return bobs(NAND32_U);
}

#endif	/* INCLUDED_ctl_dfp754_h_ */
