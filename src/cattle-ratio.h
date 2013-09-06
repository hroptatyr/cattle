/*** cattle-ratio.h -- ratios
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
#if !defined INCLUDED_cattle_ratio_h_
#define INCLUDED_cattle_ratio_h_

/**
 * Ratios, as used by newo and adex fields, unitless. */
typedef struct ctl_ratio_s ctl_ratio_t;

struct ctl_ratio_s {
	signed int p;
	unsigned int q;
};


/* helpers */
/**
 * Return the canonicalised ratio. */
extern ctl_ratio_t ctl_ratio_canon(ctl_ratio_t);

/**
 * Return the reciprocal ratio of X. */
static __inline __attribute__((const, pure)) ctl_ratio_t
ctl_ratio_recipr(ctl_ratio_t x)
{
	ctl_ratio_t res;

	if (x.p >= 0) {
		res.p = x.q;
		res.q = x.p;
	} else {
		res.p = -x.q;
		res.q = -x.p;
	}
	return res;
}

/**
 * Return the composition of two ratios. */
static __inline __attribute__((const, pure)) ctl_ratio_t
ctl_ratio_compos(ctl_ratio_t x, ctl_ratio_t y)
{
	ctl_ratio_t res = {
		.p = x.p * y.p,
		.q = x.q * y.q,
	};
	/* canonicalise? */
	return ctl_ratio_canon(res);
}

#endif	/* INCLUDED_cattle_ratio_h_ */
