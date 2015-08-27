/*** cattle.c -- tool to apply corporate actions
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <math.h>
#include "cattle.h"
#include "caev.h"
#include "caev-rdr.h"
#include "caev-wrr.h"
#include "ctl-dfp754.h"
#include "nifty.h"
#include "dt-strpf.h"
#include "caev-series.h"
#include "coru.h"

#if !defined MAP_ANON && defined MAP_ANONYMOUS
# define MAP_ANON	MAP_ANONYMOUS
#endif	/* !MAP_ANON && MAP_ANONYMOUS */

struct ctl_ctx_s {
	ctl_caevs_t q;

	unsigned int rev:1;
	unsigned int fwd:1;
	/* use absolute precision */
	unsigned int abs_prec:1;
	/* generic all flag */
	unsigned int all:1;
	/* whether to print the xdte first */
	unsigned int xdt:1;

	/* use prec fractional digits if abs_prec */
	signed int prec;
};

static char pr_buf[4096U];


static void
__attribute__((format(printf, 1, 2)))
error(const char *fmt, ...)
{
	va_list vap;
	va_start(vap, fmt);
	vfprintf(stderr, fmt, vap);
	va_end(vap);
	if (errno) {
		fputc(':', stderr);
		fputc(' ', stderr);
		fputs(strerror(errno), stderr);
	}
	fputc('\n', stderr);
	return;
}

static float
ratio_to_float(ctl_ratio_t r)
{
	if (r.p) {
		return (float)r.p / (float)r.q;
	}
	return 1.f;
}


/* coroutines */
static struct echs_fund_s
massage_rdr(const struct ctl_co_rdr_res_s *msg)
{
/* massage a message from the rdr coru into something more useful */
	struct echs_fund_s res;
	const char *p = msg->ln - 1U;

	res.t = msg->t;
	res.nf = 0U;
	for (size_t i = 0U; i < countof(res.f) && *p; i++) {
		char *next;
		res.f[res.nf++] = strtod32(p + 1U, &next);
		p = next;
	}
	return res;
}

declcoru(co_appl_pop, {
		ctl_caevs_t q;
	}, {});

static const struct pop_res_s {
	echs_instant_t t;
	ctl_kvv_t msg;
} *defcoru(co_appl_pop, c, UNUSED(arg))
{
	/* we'll yield a pop_res_s */
	struct pop_res_s res;

	while (!echs_nul_instant_p(res.t = ctl_caevs_top_rank(c->q))) {
		/* assign colour value */
		res.msg = ctl_caevs_pop(c->q);
		yield(res);
	}
	return 0;
}

declcoru(co_appl_adj, {
		bool totret;
	}, {
		const struct echs_fund_s *rdr;
		union {
			const ctl_caev_t *c;
			float f;
		} adj_param;
	});

static const struct echs_fund_s*
defcoru(co_appl_adj, ia, arg)
{
/* adjust values in RDR according to ADJ_PARAM */
	const bool totret = ia->totret;
	/* we'll yield a echs_fund_s */
	struct echs_fund_s res;
	size_t nf;

	if (UNLIKELY(arg == NULL)) {
		return 0;
	} else if ((nf = res.nf = arg->rdr->nf) == 0U || nf > 3U) {
		return 0;
	}

	if (!totret) {
		switch (nf) {
		case 1U:
			do {
				const ctl_caev_t *c = arg->adj_param.c;
				ctl_price_t adj;

				assert(c != NULL);
				adj = ctl_caev_act_mktprc(*c, arg->rdr->f[0U]);
				res.t = arg->rdr->t;
				res.f[0U] = adj;
			} while ((arg = yield(res)) != NULL);
			break;
		case 2U:
		case 3U:
			do {
				const ctl_caev_t *c = arg->adj_param.c;
				ctl_fund_t fnd;
				ctl_fund_t adj;

				assert(c != NULL);

				fnd.mktprc = arg->rdr->f[0U];
				fnd.nomval = arg->rdr->f[nf - 2U];
				fnd.outsec = arg->rdr->f[nf - 1U];

				adj = ctl_caev_act(*c, fnd);
				res.t = arg->rdr->t;
				res.f[nf - 2U] = adj.nomval;
				res.f[nf - 1U] = adj.outsec;
				res.f[0U] = adj.mktprc;
			} while ((arg = yield(res)) != NULL);
			break;
		}

	} else /* if (totret)*/ {
		switch (nf) {
		case 1U:
			do {
				const float fctr = arg->adj_param.f;

				res.t = arg->rdr->t;
				res.f[0U] = (float)arg->rdr->f[0U] * fctr;
			} while ((arg = yield(res)) != NULL);
			break;
		case 2U:
		case 3U:
			do {
				const float fctr = arg->adj_param.f;

				res.t = arg->rdr->t;
				res.f[nf - 2U] =
					(float)arg->rdr->f[nf - 2U] * fctr;
				res.f[nf - 1U] =
					(float)arg->rdr->f[nf - 1U] / fctr;
				res.f[0U] = (float)arg->rdr->f[0U] * fctr;
			} while ((arg = yield(res)) != NULL);
			break;
		}
	}

	return 0;
}

static struct echs_fund_s
massage_adj(const struct echs_fund_s *msg)
{
/* massage a message from the adj coru into something more useful */
	return *msg;
}


static int
ctl_appl_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total payout adjustment */
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	ctl_caev_t sum;
	FILE *f;

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = false);
	wrr = make_coru(ctl_co_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	if (!ctx->fwd) {
		sum = ctl_caev_sum(ctx->q);
		if (ctx->rev) {
			sum = ctl_caev_inv(sum);
		}
	} else {
		sum = ctl_zero_caev();
	}

	const struct pop_res_s *ev;
	const struct ctl_co_rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev = ctl_kvv_get_caev(ev->msg);

			/* compute the new sum */
			if (!ctx->rev) {
				/* always use left subtraction */
				sum = ctl_caev_sup(sum, caev);
			} else /*if (ctx->rev)*/ {
				/* -R mode is always just an ordinary sum */
				sum = ctl_caev_add(sum, caev);
			}
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r,
						.adj_param.c = &sum));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				pack_args(ctl_co_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));
	}
	/* drain the adjuster, then the writer */
	(void)next(adj);
	(void)next(wrr);

	free_coru(rdr);
	free_coru(pop);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	fclose(f);
	return 0;
}

static int
ctl_fadj_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return forward adjustment */
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	float prod;
	int res = 0;
	FILE *f;

	assert(ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(ctl_co_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	/* initialise product */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct ctl_co_rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev;
			float fctr;
			float aadj;

			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			caev = ctl_kvv_get_caev(ev->msg);
			aadj = (float)caev.mktprc.a;

			if (UNLIKELY(ctx->rev)) {
				/* last price needs adaption */
				last *= prod;
			}
			fctr = 1.f + aadj / last;
			fctr *= ratio_to_float(caev.mktprc.r);
			if (!ctx->rev) {
				prod /= fctr;
			} else {
				prod *= fctr;
			}
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			/* save last value */
			last = (float)r.f[0U];

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r, .adj_param.f = prod));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				pack_args(ctl_co_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));
	}
	/* unload the writer and adjuster */
	(void)next(adj);
	(void)next(wrr);

out:
	/* finished, yay */
	free_coru(pop);
	free_coru(rdr);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	fclose(f);
	return res;
}

static int
ctl_badj_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return backward adjustment */
	struct fa_s {
		echs_instant_t t;
		float fctr;
		float aadj;
		float last;
	};
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	struct fa_s *fa = NULL;
	size_t nfa = 0U;
	float prod;
	int res = 0;
	FILE *f;

	assert(!ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f, .loop = 1U);
	pop = make_coru(co_appl_pop, ctx->q);

	/* initialise another wheap and another prod */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct ctl_co_rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* skip over events from the past,
		 * i.e. from before the time of the first market price */
		for (; LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop));

		do {
			/* no need to use the strtod32() reader */
			last = strtof(ln->ln, NULL);
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     (UNLIKELY(ln == NULL) ||
			      UNLIKELY(!echs_instant_lt_p(ln->t, ev->t)));
		     ev = next(pop)) {
			ctl_caev_t caev = ctl_kvv_get_caev(ev->msg);
			struct fa_s cell;

			cell.last = last;
			cell.aadj = (float)caev.mktprc.a;
			cell.t = ev->t;

			/* represent everything as factor */
			if (LIKELY(!ctx->rev)) {
				/* all's good */
				cell.fctr = 1.f + cell.aadj / cell.last;
			} else {
				/* we do the fix up later ...
				 * there's nothing else we need */
				cell.fctr = 1.f;
			}
			cell.fctr *= ratio_to_float(caev.mktprc.r);
			prod *= cell.fctr;

			/* push to fa array */
			if ((nfa % 64U) == 0U) {
				fa = realloc(fa, (nfa + 64) * sizeof(*fa));
			}
			fa[nfa++] = cell;
		}
	}

	/* end of first pass */
	free_coru(pop);

	if (UNLIKELY(ctx->rev)) {
		/* massage the factors,
		 * we know the last factor is correct, we don't know
		 * about the N-1 factors before that though
		 * so we reinstantiate the raw price P[N-1] using the
		 * last factor and improve the (N-1)-th factor
		 * and so on */
		for (size_t i = nfa - 1U; i < nfa; i--) {
			float p = 1.f;

			/* determine sub-prod */
			for (size_t j = i + 1U; j < nfa; j++) {
				p *= fa[j].fctr;
			}
			/* unadjust i-th last price */
			fa[i].last *= p;
			/* improve i-th factor now, store inverse,
			 * use identity 1 + d/x = (x + d) / x */
			fa[i].fctr = (fa[i].last - fa[i].aadj) / fa[i].last /
				fa[i].fctr;
		}

		/* (re)calc prod */
		prod = 1.f;
		for (size_t i = 0; i < nfa; i++) {
			prod *= fa[i].fctr;
		}
	}

	/* last pass, we reuse the RDR coru as it's in loop mode */
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(ctl_co_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	last = NAN;
	size_t i;
	for (ln = next(rdr), i = 0U; ln != NULL;) {
		/* mul up factors in between price lines */
		for (;
		     LIKELY(i < nfa) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, fa[i].t));
		     i++) {
			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			/* compute the new adjustment factor */
			prod /= fa[i].fctr;
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			/* save last value */
			last = (float)r.f[0U];

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r,
						.adj_param.f = prod));
				a = massage_adj(tmp);
			}

			/* off to the writer */
			next_with(
				wrr,
				pack_args(ctl_co_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(i >= nfa || echs_instant_lt_p(ln->t, fa[i].t)));
	}
	/* unload the writer and adjuster */
	(void)next(adj);
	(void)next(wrr);

out:
	/* finished, yay */
	free_coru(rdr);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	if (nfa > 0U) {
		assert(fa != NULL);
		free(fa);
	}

	fclose(f);
	return res;
}


static int
ctl_bexp_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return backward adjustment expressed in
 * multiplicative caev echs lines */
	coru_t rdr;
	coru_t pop;
	int res = 0;
	FILE *f;

	assert(!ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);

	ctl_price_t last = nand32(NULL);
	const struct pop_res_s *ev;
	const struct ctl_co_rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* skip over events from the past,
		 * i.e. from before the time of the first market price */
		for (; LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop));

		do {
			/* no need to use the strtod32() reader */
			last = strtod32(ln->ln, NULL);
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     (UNLIKELY(ln == NULL) ||
			      UNLIKELY(!echs_instant_lt_p(ln->t, ev->t)));
		     ev = next(pop)) {
			char *bp = pr_buf;
			const char *const ep = pr_buf + sizeof(pr_buf);
			ctl_kvv_t kvv = ev->msg;
			ctl_caev_t caev = ctl_kvv_get_caev(ev->msg);
			bool rawify = false;

			if (caev.mktprc.a != 0.df) {
				ctl_ratio_t r =
					ctl_price_return(
						last + caev.mktprc.a, last);

				if (LIKELY(ctl_ratio_zero_p(caev.mktprc.r))) {
					caev.mktprc.r = r;
				} else {
					caev.mktprc.r =
						ctl_ratio_compos(
							caev.mktprc.r, r);
				}
				caev.mktprc.a = 0.df;
				rawify = true;
			}

			/* print caev */
			if (ctx->xdt) {
				bp += dt_strf(bp, ep - bp, ev->t);
				*bp++ = '\t';
			}
			if (rawify) {
				bp += ctl_caev_wrr(bp, ep - bp, ev->t, caev);
			} else {
				bp += ctl_kv_wrr(bp, ep - bp, ev->t, kvv);
			}
			*bp++ = '\n';
			*bp = '\0';
			fputs(pr_buf, stdout);
		}
	}


	/* finished, yay */
	free_coru(rdr);
	free_coru(pop);
	fini_coru();

	fclose(f);
	return res;
}

static int
ctl_blog_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return backward adjustment expressed in
 * additive caev echs lines */
	coru_t rdr;
	coru_t pop;
	int res = 0;
	FILE *f;

	assert(!ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);

	ctl_price_t last = nand32(NULL);
	const struct pop_res_s *ev;
	const struct ctl_co_rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* skip over events from the past,
		 * i.e. from before the time of the first market price */
		for (; LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop));

		do {
			/* no need to use the strtod32() reader */
			last = strtod32(ln->ln, NULL);
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     (UNLIKELY(ln == NULL) ||
			      UNLIKELY(!echs_instant_lt_p(ln->t, ev->t)));
		     ev = next(pop)) {
			char *bp = pr_buf;
			const char *const ep = pr_buf + sizeof(pr_buf);
			ctl_kvv_t v = ev->msg;
			ctl_caev_t caev;
			bool rawify = false;

			if (UNLIKELY(ctx->all)) {
				;
			} else if (ctl_kvv_get_caev_code(v) != CTL_CAEV_CTL1) {
				goto prnt;
			}

			caev = ctl_kvv_get_caev(v);
			if (!ctl_ratio_zero_p(caev.mktprc.r)) {
				ctl_price_t x = last;

				x *= caev.mktprc.r.p;
				x /= caev.mktprc.r.q;
				x -= last;

				if (LIKELY(caev.mktprc.a == 0.df)) {
					caev.mktprc.a = x;
				} else {
					caev.mktprc.a += x;
				}
				caev.mktprc.r = ctl_zero_ratio();
				rawify = true;
			}

		prnt:
			/* print caev */
			if (ctx->xdt) {
				bp += dt_strf(bp, ep - bp, ev->t);
				*bp++ = '\t';
			}
			if (rawify) {
				bp += ctl_caev_wrr(bp, ep - bp, ev->t, caev);
			} else {
				bp += ctl_kv_wrr(bp, ep - bp, ev->t, v);
			}
			*bp++ = '\n';
			*bp = '\0';
			fputs(pr_buf, stdout);
		}
	}


	/* finished, yay */
	free_coru(rdr);
	free_coru(pop);
	fini_coru();

	fclose(f);
	return res;
}

/* printer commands */
static int
ctl_print_raw(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t prev = ctl_zero_caev();
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_caevs_top_rank(ctx->q)); prev_t = t) {
		ctl_kvv_t msg = ctl_caevs_pop(ctx->q);
		ctl_caev_t this = ctl_kvv_get_caev(msg);
		char *bp = pr_buf;
		const char *const ep = pr_buf + sizeof(pr_buf);

		if (uniqp) {
			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_equal_p(this, prev)) {
				/* completely identical */
				continue;
			}
		}
		/* keep track of this caev for the next uniquify run */
		prev = this;

		if (ctx->rev) {
			this = ctl_caev_inv(this);
		}

		if (ctx->xdt) {
			bp += dt_strf(bp, ep - bp, t);
			*bp++ = '\t';
		}
		bp += ctl_caev_wrr(bp, ep - bp, t, this);
		*bp++ = '\n';
		*bp = '\0';
		fputs(pr_buf, stdout);
	}
	return res;
}

static int
ctl_print_sum(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t sum = ctl_zero_caev();
	ctl_caev_t prev = sum;
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_caevs_top_rank(ctx->q)); prev_t = t) {
		ctl_kvv_t msg = ctl_caevs_pop(ctx->q);
		ctl_caev_t this = ctl_kvv_get_caev(msg);

		if (uniqp) {
			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_equal_p(this, prev)) {
				/* completely identical */
				continue;
			}
		}
		/* keep track of this caev for the next uniquify run */
		prev = this;

		if (ctx->rev) {
			this = ctl_caev_inv(this);
		}

		/* just sum them up here */
		sum = ctl_caev_add(sum, this);
	}
	/* and now print */
	{
		char *bp = pr_buf;
		const char *const ep = pr_buf + sizeof(pr_buf);

		bp += ctl_caev_wrr(bp, ep - bp, echs_nul_instant(), sum);
		*bp++ = '\n';
		*bp = '\0';
		fputs(pr_buf, stdout);
	}
	return res;
}

static int
ctl_print_kv(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t prev_c = ctl_zero_caev();
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_caevs_top_rank(ctx->q)); prev_t = t) {
		ctl_kvv_t this = ctl_caevs_pop(ctx->q);
		char *bp = pr_buf;
		const char *const ep = pr_buf + sizeof(pr_buf);

		if (uniqp) {
			ctl_caev_t this_c = ctl_kvv_get_caev(this);

			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_equal_p(this_c, prev_c)) {
				continue;
			}
			/* store actor representation for the next round */
			prev_c = this_c;
		}

		if (ctx->xdt) {
			bp += dt_strf(bp, ep - bp, t);
			*bp++ = '\t';
		}
		bp += ctl_kv_wrr(bp, ep - bp, t, this);
		ctl_free_kvv(this);
		*bp++ = '\n';
		*bp = '\0';
		fputs(pr_buf, stdout);
	}
	return res;
}


/* helpers */
ctl_ratio_t
ctl_price_return(ctl_price_t a, ctl_price_t b)
{
/* return a / b */
	bcd32_t da = decompd32(a);
	bcd32_t db = decompd32(b);
	int sign = da.sign ^ db.sign;
	signed int p = da.mant;
	unsigned int q = db.mant;

	if (UNLIKELY(sign)) {
		p = -p;
	}
	for (; da.expo > db.expo; da.expo--) {
		p *= 10;
	}
	for (; da.expo < db.expo; da.expo++) {
		q *= 10;
	}
	return ctl_ratio_canon((ctl_ratio_t){p, q});
}


#if defined STANDALONE
#include "cattle.yucc"

static int
cmd_print(const struct yuck_cmd_print_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	bool rawp = argi->raw_flag;
	bool uniqp = argi->unique_flag;
	int rc = 1;

	if (UNLIKELY((ctx->q = make_ctl_caevs()) == NULL)) {
		goto out;
	}

	if (argi->summary_flag) {
		/* --summary implies --raw */
		rawp = true;
	} else if (argi->reverse_flag) {
		/* --reverse implies --raw */
		rawp = true;
	}

	if (argi->nargs == 0U) {
		if (UNLIKELY(ctl_read_caevs(ctx->q, NULL) < 0)) {
			error("Error: cannot read from stdin");
			goto out;
		}
	}
	for (size_t i = 0U; i < argi->nargs; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caevs(ctx->q, fn) < 0)) {
			error("Warning: cannot open file `%s'", fn);
		}
	}
	/* set flags */
	ctx->xdt = argi->xdte_flag;
	ctx->rev = argi->reverse_flag;

	if (!rawp && ctl_print_kv(ctx, uniqp) >= 0) {
		rc = 0;
	} else if (argi->summary_flag && ctl_print_sum(ctx, uniqp) >= 0) {
		rc = 0;
	} else if (rawp && ctl_print_raw(ctx, uniqp) >= 0) {
		rc = 0;
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_caevs(ctx->q);
	}
	return rc;
}

static int
cmd_apply(const struct yuck_cmd_apply_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	int rc = 0;

	if (argi->nargs < 1U) {
		yuck_auto_help((const void*)argi);
		rc = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_caevs()) == NULL)) {
		rc = 1;
		goto out;
	}

	if (argi->reverse_flag) {
		ctx->rev = 1U;
	}
	if (argi->forward_flag) {
		ctx->fwd = 1U;
	}
	if (argi->precision_arg) {
		const char *p = argi->precision_arg;
		char *on;

		if (*p == '+' || *p == '-') {
			;
		} else {
			ctx->abs_prec = 1U;
		}
		if ((ctx->prec = -strtol(p, &on, 10), *on)) {
			error("Error: invalid precision `%s'", p);
			rc = 1;
			goto out;
		}
	}

	/* open caev files and read */
	if (argi->nargs <= 1U) {
		if (UNLIKELY(ctl_read_caevs(ctx->q, NULL) < 0)) {
			error("Error: cannot read from stdin");
			rc = 1;
			goto out;
		}
	}
	for (size_t i = 1U; i < argi->nargs; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caevs(ctx->q, fn) < 0)) {
			error("Warning: cannot open caev file `%s'", fn);
		}
	}

	/* open time series file */
	with (const char *tser_fn = argi->args[0U]) {
		if (argi->total_return_flag && !ctx->fwd) {
			/* total return back adjustment needs 2 scans */
			if (UNLIKELY(ctl_badj_caev_file(ctx, tser_fn) < 0)) {
				error("\
Error: cannot deduce factors for total return adjustment from `%s'", tser_fn);
				rc = 1;
				goto out;
			}
		} else if (argi->total_return_flag/* && ctx->fwd */) {
			if (UNLIKELY(ctl_fadj_caev_file(ctx, tser_fn) < 0)) {
				error("\
Error: cannot deduce factors for total return adjustment from `%s'", tser_fn);
				rc = 1;
				goto out;
			}
		} else if (UNLIKELY(ctl_appl_caev_file(ctx, tser_fn) < 0)) {
			error("Error: cannot open series file `%s'", tser_fn);
			rc = 1;
			goto out;
		}
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_caevs(ctx->q);
		ctx->q = NULL;
	}
	return rc;
}

static int
cmd_exp(const struct yuck_cmd_exp_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	int rc = 0;

	if (argi->nargs < 1U) {
		yuck_auto_help((const void*)argi);
		rc = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_caevs()) == NULL)) {
		rc = 1;
		goto out;
	}

	/* sanrf some options */
	ctx->xdt = argi->xdte_flag;

	/* open caev files and read */
	if (argi->nargs <= 1U) {
		if (UNLIKELY(ctl_read_caevs(ctx->q, NULL) < 0)) {
			error("Error: cannot read from stdin");
			rc = 1;
			goto out;
		}
	}
	for (size_t i = 1U; i < argi->nargs; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caevs(ctx->q, fn) < 0)) {
			error("Warning: cannot open caev file `%s'", fn);
		}
	}

	/* sanrf some options */
	ctx->xdt = argi->xdte_flag;

	/* open time series file */
	with (const char *tser_fn = argi->args[0U]) {
		/* total return back adjustment needs 2 scans */
		if (UNLIKELY(ctl_bexp_caev_file(ctx, tser_fn) < 0)) {
			error("\
Error: cannot deduce factors for total return adjustment from `%s'", tser_fn);
			rc = 1;
			goto out;
		}
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_caevs(ctx->q);
		ctx->q = NULL;
	}
	return rc;
}

static int
cmd_log(const struct yuck_cmd_log_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	int rc = 0;

	if (argi->nargs < 1U) {
		yuck_auto_help((const void*)argi);
		rc = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_caevs()) == NULL)) {
		rc = 1;
		goto out;
	}

	if (argi->all_flag) {
		ctx->all = 1U;
	}

	/* open caev files and read */
	if (argi->nargs <= 1U) {
		if (UNLIKELY(ctl_read_caevs(ctx->q, NULL) < 0)) {
			error("Error: cannot read from stdin");
			rc = 1;
			goto out;
		}
	}
	for (size_t i = 1U; i < argi->nargs; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caevs(ctx->q, fn) < 0)) {
			error("Warning: cannot open caev file `%s'", fn);
		}
	}

	/* open time series file */
	with (const char *tser_fn = argi->args[0U]) {
		/* total return back adjustment needs 2 scans */
		if (UNLIKELY(ctl_blog_caev_file(ctx, tser_fn) < 0)) {
			error("\
Error: cannot deduce factors for total return adjustment from `%s'", tser_fn);
			rc = 1;
			goto out;
		}
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_caevs(ctx->q);
		ctx->q = NULL;
	}
	return rc;
}

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 99;
		goto out;
	}

	/* get the coroutines going */
	init_coru_core();

	/* check the command */
	switch (argi->cmd) {
	default:
	case CATTLE_CMD_NONE:
		error("Error: No valid command specified.\n\
See --help to obtain a list of available commands.");
		rc = 1;
		break;

	case CATTLE_CMD_APPLY:
		rc = cmd_apply((const void*)argi);
		break;

	case CATTLE_CMD_PRINT:
		rc = cmd_print((const void*)argi);
		break;

	case CATTLE_CMD_EXP:
		rc = cmd_exp((const void*)argi);
		break;

	case CATTLE_CMD_LOG:
		rc = cmd_log((const void*)argi);
		break;
	}

out:
	/* just to make sure */
	fflush(stdout);
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* cattle.c ends here */
