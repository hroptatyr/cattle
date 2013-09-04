/*** caev.h -- corporate action events
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
#if !defined INCLUDED_caev_h_
#define INCLUDED_caev_h_

#include "cattle.h"

/**
 * Cattle'll fiddle with 3 measures directly
 * - market price per security
 * - nominal value of a security
 * - outstanding securities
 * so they get their own types, see below. */

/**
 * Market determined price of a security. */
typedef ctl_price_t ctl_mktprc_t;

/**
 * Issuer determined price of a security. */
typedef ctl_price_t ctl_nomval_t;

/**
 * Number of outstanding securities. */
typedef ctl_quantity_t ctl_outsec_t;

/**
 * A corporate action event to us is a 3-tuple of linear operators,
 * one for the market price, one for the nominal value and one for the
 * outstanding shares. */
typedef struct ctl_caev_s ctl_caev_t;

/* actual layouts */
typedef struct {
	ctl_ratio_t r;
	ctl_price_t a;
} __rp_actor_t;

typedef struct {
	ctl_ratio_t r;
	ctl_quantity_t a;
} __rq_actor_t;

struct ctl_caev_s {
	__rp_actor_t mktprc;
	__rp_actor_t nomval;
	__rq_actor_t outsec;
};


/* caev opers */
/**
 * Return the joint corporate action event by applying X first, then Y. */
extern ctl_caev_t ctl_caev_add(ctl_caev_t x, ctl_caev_t y);

/**
 * Return the joint corporate action event by reversing Y with respect to X. */
extern ctl_caev_t ctl_caev_sub(ctl_caev_t x, ctl_caev_t y);

/**
 * Return the reverse corporate action event of X. */
extern ctl_caev_t ctl_caev_rev(ctl_caev_t x);

#endif	/* INCLUDED_caev_h_ */
