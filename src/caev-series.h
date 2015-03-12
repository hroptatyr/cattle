/*** caev-series.h -- time series of caevs
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
 **/
#if !defined INCLUDED_caev_series_h_
#define INCLUDED_caev_series_h_
#include <stdio.h>
#include "caev.h"
#include "coruaux.h"

typedef union {
	ctl_caev_t c;
	void *flds;
} colour_t;
#define WHEAP_COLOUR_T

typedef struct ctl_wheap_s *ctl_caevs_t;

/* must be included after we've def'd WHEAP_COLOUR_T */
#include "wheap.h"

struct echs_fund_s {
	echs_instant_t t;
	size_t nf;
	_Decimal32 f[3U];
};

/* as a service we expose the actual coroutine details here */
declcoru(ctl_co_rdr, {
		FILE *f;
		/* after EOF rewind to the beginning and start over */
		size_t loop;
	}, {});

extern const struct ctl_co_rdr_res_s {
	echs_instant_t t;
	const char *ln;
	size_t lz;
} *defcoru(ctl_co_rdr, c, arg);

declcoru(ctl_co_wrr, {
		bool absp;
		signed int prec;
	}, {
		const struct echs_fund_s *rdr;
		const struct echs_fund_s *adj;
	});

extern const void *defcoru(ctl_co_wrr, ia, arg);


/**
 * Return a sum of corporate actions without changing the contents of CS. */
extern ctl_caev_t ctl_caev_sum(ctl_caevs_t cs);


/* coroutine-powered high level routines */
/**
 * Read CAEVs from FN and put them in Q. */
extern int ctl_read_caevs(ctl_caevs_t q, const char *fn);


static inline ctl_caevs_t
make_ctl_caevs(void)
{
	return make_ctl_wheap();
}

static inline void
free_ctl_caevs(ctl_caevs_t q)
{
	return free_ctl_wheap(q);
}

#define ctl_caevs_top_rank	ctl_wheap_top_rank
#define ctl_caevs_pop		ctl_wheap_pop
#define ctl_caevs_add		ctl_wheap_add
#define ctl_caevs_add_deferred	ctl_wheap_add_deferred
#define ctl_caevs_fix_deferred	ctl_wheap_fix_deferred

#endif	/* INCLUDED_caev_series_h_ */
