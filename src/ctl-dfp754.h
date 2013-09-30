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

extern int d32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtod32(const char*, char**);

extern int bid32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtobid32(const char*, char**);

extern int dpd32tostr(char *restrict buf, size_t bsz, _Decimal32);
extern _Decimal32 strtodpd32(const char*, char**);


static inline __attribute__((pure, const)) uint32_t
bits(_Decimal32 x)
{
	return (union {_Decimal32 x; uint32_t u;}){x}.u;
}

static inline __attribute__((pure, const)) _Decimal32
bobs(uint32_t u)
{
	return (union {uint32_t u; _Decimal32 x;}){u}.x;
}

#endif	/* INCLUDED_ctl_dfp754_h_ */