/*** caev.c -- corporate action events
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
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <tgmath.h>
#include "cattle.h"
#include "caev.h"
#include "nifty.h"

typedef __rp_actor_t rp_t;
typedef __rq_actor_t rq_t;

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


static __attribute__((const, pure)) ctl_price_t
price_add(ctl_price_t x, ctl_price_t y)
{
	return x + y;
}

static __attribute__((const, pure)) ctl_quant_t
quant_add(ctl_quant_t x, ctl_quant_t y)
{
	return x + y;
}

static __attribute__((const, pure)) ctl_ratio_t
ratio_canon(ctl_ratio_t x)
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

static __attribute__((const, pure)) ctl_ratio_t
ratio_add(ctl_ratio_t x, ctl_ratio_t y)
{
	ctl_ratio_t res = {
		.p = x.p * y.p,
		.q = x.q * y.q,
	};
	/* canonicalise? */
	return ratio_canon(res);
}

static __attribute__((const, pure)) ctl_price_t
price_rev(ctl_price_t x)
{
	return -x;
}

static __attribute__((const, pure)) ctl_quant_t
quant_rev(ctl_quant_t x)
{
	return -x;
}

static __attribute__((const, pure)) ctl_ratio_t
ratio_rev(ctl_ratio_t x)
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


static rp_t
rp_add(rp_t x, rp_t y)
{
	rp_t res = {
		.r = ratio_add(x.r, y.r),
		.a = price_add(x.a, y.a),
	};
	return res;
}

static rq_t
rq_add(rq_t x, rq_t y)
{
	rq_t res = {
		.r = ratio_add(x.r, y.r),
		.a = quant_add(x.a, y.a),
	};
	return res;
}

static rp_t
rp_rev(rp_t x)
{
	rp_t res = {
		.r = ratio_rev(x.r),
		.a = price_rev(x.a),
	};
	return res;
}

static rq_t
rq_rev(rq_t x)
{
	rq_t res = {
		.r = ratio_rev(x.r),
		.a = quant_rev(x.a),
	};
	return res;
}


/* API impl */
ctl_caev_t
ctl_caev_add(ctl_caev_t x, ctl_caev_t y)
{
	ctl_caev_t res = {
		.mktprc = rp_add(x.mktprc, y.mktprc),
		.nomval = rp_add(x.nomval, y.nomval),
		.outsec = rq_add(x.outsec, y.outsec),
	};
	return res;
}

ctl_caev_t
ctl_caev_sub(ctl_caev_t x, ctl_caev_t y)
{
	ctl_caev_t res = {
		.mktprc = rp_add(x.mktprc, rp_rev(y.mktprc)),
		.nomval = rp_add(x.nomval, rp_rev(y.nomval)),
		.outsec = rq_add(x.outsec, rq_rev(y.outsec)),
	};
	return res;
}

ctl_caev_t
ctl_caev_rev(ctl_caev_t x)
{
	ctl_caev_t res = {
		.mktprc = rp_rev(x.mktprc),
		.nomval = rp_rev(x.nomval),
		.outsec = rq_rev(x.outsec),
	};
	return res;
}

/* caev.c ends here */
