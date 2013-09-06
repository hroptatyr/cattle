/*** cattle-ratio.c -- ratios
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
#include "cattle-ratio.h"
#include "nifty.h"

static ctl_ratio_t null_ratio;


/* aux */
static unsigned int
gcd(unsigned int u, unsigned int v)
{
	unsigned int shift;

	/* GCD(0,v) == v; GCD(u,0) == u, GCD(0,0) == 0 */
	if (u == 0) {
		return v;
	} else if (v == 0) {
		return u;
	}

	/* determine greatest 2-power factor in u and v */
	for (shift = 0; ((u | v) & 1) == 0U; shift++) {
		u >>= 1;
		v >>= 1;
	}

	/* cancel all further 2-powers in u */
	while ((u & 1) == 0) {
		u >>= 1;
	}

	/* u is odd. */
	do {
		/* cancel all 2-powers in v */
		while ((v & 1) == 0) {
			v >>= 1;
		}

		/* u, v are now both odd, swap s.t. u <= v
		 * then set v = v - u (which is even) */
		if (u > v) {
			unsigned int t = v;
			v = u;
			u = t;
		}
		v -= u;
	} while (v != 0);

	/* restore common 2-power */
	return u << shift;
}


ctl_ratio_t
ctl_ratio_canon(ctl_ratio_t x)
{
/* canonicalise X */
	unsigned int g;
	ctl_ratio_t res;

	if (UNLIKELY(x.p == 0)) {
		return null_ratio;
	}
	g = gcd(x.p > 0 ? x.p : -x.p, x.q);
	res = (ctl_ratio_t){
		.p = x.p / (signed int)g,
		.q = x.q / g,
	};
	return res;
}

/* cattle-ratio.c ends here */
