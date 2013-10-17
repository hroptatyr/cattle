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


static ctl_price_actor_t
price_actor_add(ctl_price_actor_t x, ctl_price_actor_t y)
{
	ctl_price_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = price_add(x.a, y.a),
		.f = x.f && y.f ? x.f * y.f : x.f ?: y.f,
	};
	return res;
}

static ctl_quant_actor_t
quant_actor_add(ctl_quant_actor_t x, ctl_quant_actor_t y)
{
	ctl_quant_actor_t res = {
		.r = ratio_add(x.r, y.r),
		.a = quant_add(x.a, y.a),
		.f = x.f && y.f ? x.f * y.f : x.f ?: y.f,
	};
	return res;
}

static ctl_price_actor_t
price_actor_rev(ctl_price_actor_t x)
{
	ctl_price_actor_t res = {
		.r = ratio_rev(x.r),
		.a = price_rev(x.a),
		.f = x.f ? 1. / x.f : 0.,
	};
	return res;
}

static ctl_price_actor_t
price_actor_absrev(ctl_price_actor_t x)
{
	x.a = price_rev(x.a);
	return x;
}

static ctl_quant_actor_t
quant_actor_rev(ctl_quant_actor_t x)
{
	ctl_quant_actor_t res = {
		.r = ratio_rev(x.r),
		.a = quant_rev(x.a),
		.f = x.f ? 1. / x.f : 0.,
	};
	return res;
}

static ctl_quant_actor_t
quant_actor_absrev(ctl_quant_actor_t x)
{
	x.a = quant_rev(x.a);
	return x;
}

static ctl_price_t
price_act(ctl_price_actor_t a, ctl_price_t x)
{
	ctl_price_t res = x + a.a;

	if (a.r.p) {
		res = (res * a.r.p) / a.r.q;
	}
	if (a.f) {
		res = res * a.f;
	}
	return res;
}

static ctl_quant_t
quant_act(ctl_quant_actor_t a, ctl_quant_t x)
{
	ctl_quant_t res = x + a.a;

	if (a.r.p) {
		res = (res * a.r.p) / a.r.q;
	}
	if (a.f) {
		res = res * a.f;
	}
	return res;
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
ctl_caev_absrev(ctl_caev_t x)
{
	ctl_caev_t res = {
		.mktprc = price_actor_absrev(x.mktprc),
		.nomval = price_actor_absrev(x.nomval),
		.outsec = quant_actor_absrev(x.outsec),
	};
	return res;
}

ctl_caev_t
ctl_caev_zero_abs(ctl_caev_t x)
{
	x.mktprc.a = ctl_zero_price();
	x.mktprc.a = ctl_zero_price();
	x.outsec.a = ctl_zero_quant();
	return x;
}

ctl_caev_t
ctl_caev_rel(ctl_caev_t x, ctl_fund_t f)
{
	double fctr;

	if (x.mktprc.a) {
		fctr = f.mktprc / (f.mktprc + x.mktprc.a);
		x.mktprc.f = x.mktprc.f ? x.mktprc.f * fctr : fctr;
		x.mktprc.a = ctl_zero_price();
	}

	if (x.nomval.a) {
		fctr = f.nomval / (f.nomval + x.nomval.a);
		x.nomval.f = x.nomval.f ? x.nomval.f * fctr : fctr;
		x.nomval.a = ctl_zero_price();
	}

	if (x.outsec.a) {
		fctr = f.outsec / (f.outsec + x.outsec.a);
		x.outsec.f = x.outsec.f ? x.outsec.f * fctr : fctr;
		x.outsec.a = ctl_zero_quant();
	}
	return x;
}

ctl_caev_t
ctl_caev_rel_mktprc(ctl_caev_t x, ctl_price_t p)
{
	if (x.mktprc.a) {
		double fctr = (double)p / ((double)p + (double)x.mktprc.a);
		x.mktprc.f = x.mktprc.f ? x.mktprc.f * fctr : fctr;
		x.mktprc.a = ctl_zero_price();
	}
	return x;
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
