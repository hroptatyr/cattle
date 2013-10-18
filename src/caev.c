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
 * FIFO-sub, use ctl_caev_sup(). */


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


static ctl_price_actor_t
price_actor_add(ctl_price_actor_t x, ctl_price_actor_t y)
{
	ctl_price_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = price_add(ratio_act_p(ratio_rev(y.r), x.a), y.a),
	};
	return res;
}

static ctl_quant_actor_t
quant_actor_add(ctl_quant_actor_t x, ctl_quant_actor_t y)
{
	ctl_quant_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = quant_add(ratio_act_q(ratio_rev(y.r), x.a), y.a),
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
		.mktprc = price_actor_add(x.mktprc, price_actor_rev(y.mktprc)),
		.nomval = price_actor_add(x.nomval, price_actor_rev(y.nomval)),
		.outsec = quant_actor_add(x.outsec, quant_actor_rev(y.outsec)),
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

/* caev.c ends here */
