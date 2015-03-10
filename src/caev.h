/*** caev.h -- corporate action events
 *
 * Copyright (C) 2013-2015 Sebastian Freundt
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
#if !defined INCLUDED_caev_h_
#define INCLUDED_caev_h_

#include <stdbool.h>
#include <string.h>
#include "cattle.h"
#include "intern.h"

/**
 * Cattle'll fiddle with 3 measures directly
 * - market price per security
 * - nominal value of a security
 * - outstanding securities
 * so they get their own type, fund stands for fundamentals. */
typedef struct ctl_fund_s ctl_fund_t;

/**
 * A corporate action event to us is a 3-tuple of linear operators,
 * one for the market price, one for the nominal value and one for the
 * outstanding shares. */
typedef struct ctl_caev_s ctl_caev_t;

/**
 * Key/value representation of a corporate action message. */
typedef struct ctl_kvv_s *ctl_kvv_t;

struct ctl_kv_s {
	obint_t key;
	obint_t val;
};

struct ctl_kvv_s {
	size_t nkvv;
	struct ctl_kv_s kvv[];
};

/**
 * An actor is a parametrisation of what it takes for a corporate action
 * event to change (act on) the fundamentals. */
#define DEFACTOR(x)							\
	typedef struct ctl_##x##_actor_s ctl_##x##_actor_t;		\
	struct ctl_##x##_actor_s {					\
		ctl_ratio_t r;						\
		ctl_##x##_t a;						\
	};								\
	static inline __attribute__((pure, const)) ctl_##x##_actor_t	\
	ctl_zero_##x##_actor(void)					\
	{								\
		return (ctl_##x##_actor_t){				\
			ctl_zero_ratio(),				\
			ctl_zero_##x(),					\
		};							\
	}								\
	/* just to shut the compiler up */				\
	struct ctl_##x##_actor_s
#define ACTOR(x)	ctl_##x##_actor_t

/* actual layouts */
struct ctl_fund_s {
	ctl_price_t mktprc;
	ctl_price_t nomval;
	ctl_quant_t outsec;
};

/* defines type ctl_price_actor_t */
DEFACTOR(price);
/* defines type ctl_quant_actor_t */
DEFACTOR(quant);

struct ctl_caev_s {
	ctl_price_actor_t mktprc;
	ctl_price_actor_t nomval;
	ctl_quant_actor_t outsec;
};


/* caev opers */
/**
 * Return the joint corporate action event by applying X first, then Y.
 * This addition is *NOT* commutative, and there are 2 inverses:
 * ctl_caev_sup(SUM, Y) to obtain X, and respectively
 * ctl_caev_sub(SUM, X) to obtain Y, where SUM = ctl_caev_add(X, Y). */
extern ctl_caev_t ctl_caev_add(ctl_caev_t x, ctl_caev_t y);

/**
 * Return the joint corporate action event when taking off past event Y
 * (wrt the period captured in X this is the old-end) of X. */
extern ctl_caev_t ctl_caev_sup(ctl_caev_t x, ctl_caev_t y);

/**
 * Return the joint corporate action event when taking off event Y, off
 * X at the the young-end, or present end. */
extern ctl_caev_t ctl_caev_sub(ctl_caev_t x, ctl_caev_t y);

/**
 * Return the reverse corporate action event of X.
 * That is a corporate action event that can be added to the right
 * (present side) to yield 0+0<-0, the neutral CA event. */
extern ctl_caev_t ctl_caev_rev(ctl_caev_t x);

/**
 * Return the inverse corporate action event of X.
 * That is a corporate action event that can be added to the left
 * (past side) to yield 0+0<-0, the neutral CA event. */
extern ctl_caev_t ctl_caev_inv(ctl_caev_t x);


/* caev actions */
/**
 * Return the fundamentals after CAEV acted on X.
 * For prices it is assumed that currencies match, for quanities it is
 * assumed that the quantified objects match. */
extern ctl_fund_t ctl_caev_act(ctl_caev_t, ctl_fund_t x);

/**
 * Return the price after CAEV acted on X.
 * It is assumed that currencies match. */
extern ctl_price_t ctl_caev_act_mktprc(ctl_caev_t, ctl_price_t x);

/**
 * Return true iff C1 and C2 act equally. */
extern bool ctl_caev_equal_p(ctl_caev_t c1, ctl_caev_t c2);


static inline __attribute__((const, pure)) ctl_caev_t
ctl_zero_caev(void)
{
	return (ctl_caev_t){
		ctl_zero_price_actor(),
		ctl_zero_price_actor(),
		ctl_zero_quant_actor(),
	};
}

/**
 * Return true iff C1 and C2 are equal. */
static inline __attribute__((const, pure)) bool
ctl_caev_eq_p(ctl_caev_t c1, ctl_caev_t c2)
{
	return !memcmp(&c1, &c2, sizeof(ctl_caev_t));
}

#endif	/* INCLUDED_caev_h_ */
