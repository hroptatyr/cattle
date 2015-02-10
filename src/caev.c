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

/* NOTE:
 * The CAEV operations ADD is *NOT* generally commutative, instead you
 * must obey chronological order.  Same for SUB, the latest CAEV added
 * must be subbed first (LIFO), this is the right inverse of ADD, called
 * ctl_caev_sub();  to get the left inverse of ADD, respectively, the
 * FIFO-sub, use ctl_caev_sup().
 *
 * The arithmetic we implement here favours additive operations over
 * multiplicative.  In our (raw) syntax we will write  x+p<-q  meaning
 * add x to the price, then, if p is non-zero, multiply by p, then for
 * non-zero q divide by q.
 *
 * Add for additive operations is defined as:
 *
 *     x1+0<-0 + x2+0<-0 = (x1+x2)+0<-0
 *
 * and hence the inverse of x+0<-0 is (-x)+0<-0.
 *
 * Similarly, add for multiplicative operations is defined as:
 *
 *    0+p1<-q1 + 0+p2<-q2 = 0+(p1p2)<-(q1q2)
 *
 * which makes 0+q<-p the inverse of 0+p<-q, with 0+1<-1 being the
 * neutral element.
 *
 * For mixed operations the precedence rule means:
 *
 *     x1+p1<-q1 + x2+p2<-q2 = (x1+x2q1/p1)+(p1p2)<-(q1q2)
 *
 * So a dividend before a split becomes
 *
 *     d+0<-0 + 0+p<-q = d+p<-q
 *
 * and a split before a dividend is summed as
 *
 *     0+p<-q + d+0<-0 = (dq/p)+p<-q
 *
 * This arithmetic satisfies dilution properties just properly,
 * a dividend of 4 paid before a 2:1 split is the same as a
 * dividend of 2 paid after a 2:1 split:
 *
 *     10@20         10@20
 *     20@10         10@16 + 10*4
 *     20@8 + 20*2   20@8  + 10*4
 *     =160 + 40     =160  + 40
 *
 * And so DVCA/4 + SPLF/2:1 = SPLF/2:1 + DVCA/2 or in raw syntax
 * -4+0<-0 + 0+1<-2  = -4+1<-2  vs  0+1<-2 + -2+0<-0 = -4+1<-2
 *
 * From this the 2 inverses of + become apparent, since the additive
 * part is always absolute in value, subtraction of additive operations
 * from the left is simply an addition.
 *
 *     x1+p1<-q1 -l x2+p2<-q2 = (x1 - x2)+p1/p2<-q1/q2
 *
 * And since all multiplicative operations are aggregated into the
 * multiplicative cell of a sum subtraction of additive operations from the
 * right is defined as:
 *
 *     x1+p1<-q1 -r x2+p2<-q2 = (x1 - q2x2/p2)+p1/p2<-q1/q2
 *
 * The actual inverse elements can be computed via left or right
 * subtraction from the neutral operation 0+0<-0:
 *
 *     0+0<-0  -l  x+p<-q = (-x)+q<-p
 *
 * which is called the rev or left-inverse, and
 *
 *     0+0<-0  -r  x+p<-q = (-qx/p)+q<-p
 *
 * respectively, which we simply call inv or right-inverse. */


static __attribute__((const, pure)) ctl_price_t
price_add(ctl_price_t x, ctl_price_t y)
{
	return ctl_price_compos(x, y);
}

static __attribute__((const, pure)) ctl_quant_t
quant_add(ctl_quant_t x, ctl_quant_t y)
{
	return ctl_quant_compos(x, y);
}

static __attribute__((const, pure)) ctl_ratio_t
ratio_add(ctl_ratio_t x, ctl_ratio_t y)
{
	if (x.p == 0 && x.q == 0U) {
		return y;
	} else if (y.p == 0 && y.q == 0U) {
		return x;
	}
	return ctl_ratio_compos(x, y);
}

static __attribute__((const, pure)) ctl_price_t
price_rev(ctl_price_t x)
{
	return ctl_price_recipr(x);
}

static __attribute__((const, pure)) ctl_quant_t
quant_rev(ctl_quant_t x)
{
	return ctl_quant_recipr(x);
}

static __attribute__((const, pure)) ctl_ratio_t
ratio_rev(ctl_ratio_t x)
{
	return ctl_ratio_recipr(x);
}

static __attribute__((const, pure)) bool
ratio_equal_p(ctl_ratio_t r1, ctl_ratio_t r2)
{
	if (r1.p && r2.p) {
		return r1.p == r2.p && r1.q == r2.q;
	} else if (r1.p) {
		/* r2 is 1<-1 */
		return r1.p == 1 && r1.q == 1;
	} else if (r2.p) {
		/* r1 is 1<-1 */
		return r2.p == 1 && r2.q == 1;
	}
	/* otherwise both r1.p and r2.p are zero */
	return true;
}

static __attribute__((const, pure)) ctl_price_t
ratio_act_p(ctl_ratio_t r, ctl_price_t p)
{
	if (r.p) {
		p = (p * r.p) / r.q;
	}
	return p;
}

static __attribute__((const, pure)) ctl_quant_t
ratio_act_q(ctl_ratio_t r, ctl_quant_t q)
{
	if (r.p) {
		q = (q * r.p) / r.q;
	}
	return q;
}

static __attribute__((const, pure)) bool
price_equal_p(ctl_price_t p1, ctl_price_t p2)
{
	return p1 == p2;
}

static __attribute__((const, pure)) bool
quant_equal_p(ctl_quant_t q1, ctl_quant_t q2)
{
	return q1 == q2;
}


static ctl_price_actor_t
price_actor_add(ctl_price_actor_t x, ctl_price_actor_t y)
{
	ctl_price_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = price_add(x.a, ratio_act_p(ratio_rev(x.r), y.a)),
	};
	return res;
}

static ctl_quant_actor_t
quant_actor_add(ctl_quant_actor_t x, ctl_quant_actor_t y)
{
	ctl_quant_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = quant_add(x.a, ratio_act_p(ratio_rev(x.r), y.a)),
	};
	return res;
}

static ctl_price_actor_t
price_actor_rev(ctl_price_actor_t x)
{
	ctl_price_actor_t res = {
		.r = ratio_rev(x.r),
		.a = price_rev(x.a),
	};
	return res;
}

static ctl_quant_actor_t
quant_actor_rev(ctl_quant_actor_t x)
{
	ctl_quant_actor_t res = {
		.r = ratio_rev(x.r),
		.a = quant_rev(x.a),
	};
	return res;
}

static ctl_price_actor_t
price_actor_inv(ctl_price_actor_t x)
{
	x.a = ratio_act_p(x.r, x.a);
	return price_actor_rev(x);
}

static ctl_quant_actor_t
quant_actor_inv(ctl_quant_actor_t x)
{
	x.a = ratio_act_q(x.r, x.a);
	return quant_actor_rev(x);
}

static bool
price_actor_equal_p(ctl_price_actor_t a1, ctl_price_actor_t a2)
{
	return price_equal_p(a1.a, a2.a) &&
		ratio_equal_p(a1.r, a2.r);
}

static bool
quant_actor_equal_p(ctl_quant_actor_t a1, ctl_quant_actor_t a2)
{
	return quant_equal_p(a1.a, a2.a) &&
		ratio_equal_p(a1.r, a2.r);
}

static ctl_price_t
price_act(ctl_price_actor_t a, ctl_price_t x)
{
	return ratio_act_p(a.r, x + a.a);
}

static ctl_quant_t
quant_act(ctl_quant_actor_t a, ctl_quant_t x)
{
	return ratio_act_q(a.r, x + a.a);
}


/* API impl */
ctl_caev_t
ctl_caev_add(ctl_caev_t x, ctl_caev_t y)
{
	ctl_caev_t res = {
		.mktprc = price_actor_add(x.mktprc, y.mktprc),
		.nomval = price_actor_add(x.nomval, y.nomval),
		.outsec = quant_actor_add(x.outsec, y.outsec),
	};
	return res;
}

ctl_caev_t
ctl_caev_sub(ctl_caev_t x, ctl_caev_t y)
{
	ctl_caev_t res = {
		.mktprc = price_actor_add(x.mktprc, price_actor_inv(y.mktprc)),
		.nomval = price_actor_add(x.nomval, price_actor_inv(y.nomval)),
		.outsec = quant_actor_add(x.outsec, quant_actor_inv(y.outsec)),
	};
	return res;
}

ctl_caev_t
ctl_caev_sup(ctl_caev_t x, ctl_caev_t y)
{
	ctl_caev_t res = {
		.mktprc = price_actor_add(price_actor_rev(y.mktprc), x.mktprc),
		.nomval = price_actor_add(price_actor_rev(y.nomval), x.nomval),
		.outsec = quant_actor_add(quant_actor_rev(y.outsec), x.outsec),
	};
	return res;
}

ctl_caev_t
ctl_caev_rev(ctl_caev_t x)
{
	ctl_caev_t res = {
		.mktprc = price_actor_rev(x.mktprc),
		.nomval = price_actor_rev(x.nomval),
		.outsec = quant_actor_rev(x.outsec),
	};
	return res;
}

ctl_caev_t
ctl_caev_inv(ctl_caev_t x)
{
	ctl_caev_t res = {
		.mktprc = price_actor_inv(x.mktprc),
		.nomval = price_actor_inv(x.nomval),
		.outsec = quant_actor_inv(x.outsec),
	};
	return res;
}


/* actions */
ctl_fund_t
ctl_caev_act(ctl_caev_t e, ctl_fund_t x)
{
	ctl_fund_t res = {
		.mktprc = price_act(e.mktprc, x.mktprc),
		.nomval = price_act(e.nomval, x.nomval),
		.outsec = quant_act(e.outsec, x.outsec),
	};
	return res;
}

ctl_price_t
ctl_caev_act_mktprc(ctl_caev_t e, ctl_price_t x)
{
	return price_act(e.mktprc, x);
}


/* predicates */
bool
ctl_caev_equal_p(ctl_caev_t c1, ctl_caev_t c2)
{
	if (ctl_caev_eq_p(c1, c2)) {
		return true;
	}
	/* otherwise check if C1 and C2 act equally */
	return price_actor_equal_p(c1.mktprc, c2.mktprc) &&
		price_actor_equal_p(c1.nomval, c2.nomval) &&
		quant_actor_equal_p(c1.outsec, c2.outsec);
}

/* caev.c ends here */
