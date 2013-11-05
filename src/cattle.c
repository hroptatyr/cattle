/*** cattle.c -- tool to apply corporate actions
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
#include "ctl-dfp754.h"
#include "nifty.h"
#include "dt-strpf.h"
#include "wheap.h"
#include "coru.h"

struct ctl_ctx_s {
	ctl_wheap_t q;
	ctl_caev_t sum;

	unsigned int rev:1;
	unsigned int fwd:1;
	/* use absolute precision */
	unsigned int abs_prec:1;

	/* use prec fractional digits if abs_prec */
	signed int prec;
};

#define PACK(str, args...)	((str){args})


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

static int
pr_ei(echs_instant_t t)
{
	char buf[32U];
	return fwrite(buf, sizeof(*buf), dt_strf(buf, sizeof(buf), t), stdout);
}

static int
pr_d32(_Decimal32 x)
{
	char bf[32U];
	return fwrite(bf, sizeof(*bf), bid32tostr(bf, sizeof(bf), x), stdout);
}

static float
ratio_to_float(ctl_ratio_t r)
{
	if (r.p) {
		return (float)r.p / (float)r.q;
	}
	return 1.f;
}

static size_t
ctl_caev_wr(char *restrict buf, size_t bsz, ctl_caev_t c)
{
	static const char caev[] = "caev=CTL1";
	static const char mkti[] = ".xmkt=";
	static const char nomi[] = ".xnom=";
	static const char outi[] = ".xout=";
	char *restrict bp = buf;
	const char *const ep = buf + bsz;

#define BANG_LIT(p, ep, x)					\
	({							\
		size_t z = sizeof(x) - 1;			\
		(p + z < ep)					\
			? (memcpy(p, (x), z), z)		\
			: 0UL					\
			;					\
	})

	bp += BANG_LIT(bp, ep, caev);
	*bp++ = ' ';
	*bp++ = '{';
	bp += BANG_LIT(bp, ep, mkti);
	*bp++ = '"';
	bp += bid32tostr(bp, ep - bp, c.mktprc.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.mktprc.r.p, c.mktprc.r.q);
	*bp++ = '"';
	*bp++ = ' ';
	bp += BANG_LIT(bp, ep, nomi);
	*bp++ = '"';
	bp += bid32tostr(bp, ep - bp, c.nomval.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.nomval.r.p, c.nomval.r.q);
	*bp++ = '"';
	*bp++ = ' ';
	bp += BANG_LIT(bp, ep, outi);
	*bp++ = '"';
	bp += bid32tostr(bp, ep - bp, c.outsec.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.outsec.r.p, c.outsec.r.q);
	*bp++ = '"';
	*bp++ = '}';
	*bp = '\0';
	return bp - buf;
}

static _Decimal32
mkscal(signed int nd)
{
/* produce a d32 with -ND fractional digits */
	return scalbnd32(1.df, nd);
}


/* coroutines */
struct echs_fund_s {
	echs_instant_t t;
	size_t nf;
	_Decimal32 f[3U];
};

struct adj_in_s {
	const struct echs_fund_s *rdr;
	union {
		const ctl_caev_t *c;
		float f;
	} adj_param;
};

struct wrr_in_s {
	const struct echs_fund_s *rdr;
	const struct echs_fund_s *adj;
};

declcoru(co_appl_rdr, {
		FILE *f;
	});

static const struct rdr_res_s {
	echs_instant_t t;
	const char *ln;
	size_t lz;
} *co_appl_rdr(void *UNUSED(arg), const struct co_appl_rdr_initargs_s *c)
{
/* coroutine for the reader of the tseries */
	char *line = NULL;
	size_t llen = 0UL;
	ssize_t nrd;
	/* we'll yield a rdr_res */
	struct rdr_res_s res;

	while ((nrd = getline(&line, &llen, c->f)) > 0) {
		char *p;

		if (*line == '#') {
			continue;
		} else if (__inst_0_p(res.t = dt_strp(line, &p))) {
			continue;
		} else if (*p != '\t') {
			continue;
		}
		/* pack the result structure */
		res.ln = p + 1U;
		res.lz = nrd - (p + 1U - line);
		yield(res);
	}

	free(line);
	line = NULL;
	llen = 0U;
	return 0;
}

static struct echs_fund_s
massage_rdr(const struct rdr_res_s *msg)
{
/* massage a message from the rdr coru into something more useful */
	struct echs_fund_s res;
	const char *p = msg->ln - 1U;

	res.t = msg->t;
	res.nf = 0U;
	for (size_t i = 0U; i < countof(res.f) && *p != '\n'; i++) {
		char *next;
		res.f[res.nf++] = strtod32(p + 1U, &next);
		p = next;
	}
	return res;
}

declcoru(co_appl_pop, {
		ctl_wheap_t q;
	});

static const struct pop_res_s {
	echs_instant_t t;
	const void *msg;
	size_t msz;
} *co_appl_pop(void *UNUSED(arg), const struct co_appl_pop_initargs_s *c)
{
	/* we'll yield a pop_res_s */
	struct pop_res_s res;

	while (!__inst_0_p(res.t = ctl_wheap_top_rank(c->q))) {
		/* assume it's a ctl-caev_t */
		uintptr_t tmp = ctl_wheap_pop(c->q);
		res.msg = (const ctl_caev_t*)tmp;
		res.msz = sizeof(ctl_caev_t);
		yield(res);
	}
	return 0;
}

declcoru(co_appl_wrr, {
		bool absp;
		signed int prec;
	});

static const void*
co_appl_wrr(const struct wrr_in_s *arg, const struct co_appl_wrr_initargs_s *ia)
{
/* no yield */
	const bool absp = ia->absp;
	const signed int prec = ia->prec;

	if (!absp) {
		while (arg != NULL) {
			_Decimal32 prc = arg->rdr->f[0U];

			pr_ei(arg->adj->t);

			if (UNLIKELY(prec)) {
				/* come up with a new raw value */
				int tgtx = quantexpd32(prc) + prec;
				prc = scalbnd32(1.df, tgtx);
			}
			fputc('\t', stdout);
			pr_d32(quantized32(arg->adj->f[0U], prc));

			for (size_t i = 1U; i < arg->adj->nf; i++) {
				/* print the rest without prec scaling */
				fputc('\t', stdout);
				pr_d32(arg->adj->f[i]);
			}
			fputc('\n', stdout);
			arg = yield(NULL);
		}
	} else /*if (absp)*/ {
		const _Decimal32 scal = mkscal(prec);

		/* absolute precision mode */
		while (arg != NULL) {
			pr_ei(arg->adj->t);

			fputc('\t', stdout);
			pr_d32(quantized32(arg->adj->f[0U], scal));

			for (size_t i = 1U; i < arg->adj->nf; i++) {
				/* print the rest without prec scaling */
				fputc('\t', stdout);
				pr_d32(arg->adj->f[i]);
			}
			fputc('\n', stdout);
			arg = yield(NULL);
		}
	}
	return 0;
}

declcoru(co_appl_adj, {
		bool totret;
	});

static const struct echs_fund_s*
co_appl_adj(const struct adj_in_s *arg, const struct co_appl_adj_initargs_s *ia)
{
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
					(float)arg->rdr->f[nf - 2U] / fctr;
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


/* public api, might go to libcattle one day */
static int
ctl_read_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn */
	static ctl_caev_t *caevs;
	static size_t ncaevs;
	size_t caevi = 0U;
	coru_t rdr;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	/* initialise sum to some zero */
	ctx->sum = ctl_zero_caev();

	for (const struct rdr_res_s *ln; (ln = next(rdr));) {
		/* try to read the whole shebang */
		ctl_caev_t c = ctl_caev_rdr(ctx, ln->t, ln->ln);
		uintptr_t qmsg;

		/* resize check */
		if (caevi >= ncaevs) {
			size_t nu = ncaevs + 64U;
			caevs = realloc(caevs, nu * sizeof(*caevs));
			ncaevs = nu;
		}

		/* `clone' C */
		qmsg = (uintptr_t)(caevs + caevi);
		caevs[caevi++] = c;
		/* insert to heap */
		ctl_wheap_add_deferred(ctx->q, ln->t, qmsg);
		/* also sum them up */
		ctx->sum = ctl_caev_add(ctx->sum, c);
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(ctx->q);
	free_coru(rdr);
	fclose(f);
	fini_coru();
	return 0;
}

static int
ctl_appl_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices */
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	ctl_caev_t sum;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = false);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	if (!ctx->fwd && !ctx->rev) {
		sum = ctx->sum;
	} else if (!ctx->rev/* && ctx->fwd */) {
		sum = ctl_zero_caev();
	} else if (!ctx->fwd/* && ctx->rev */) {
		sum = ctl_caev_inv(ctx->sum);
	} else /*if (ctx->fwd && ctx->rev)*/ {
		sum = ctl_zero_caev();
	}

	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev;

			caev = *(const ctl_caev_t*)ev->msg;

			/* compute the new sum */
			if (!ctx->fwd && !ctx->rev) {
				sum = ctl_caev_sup(sum, caev);
			} else if (!ctx->rev/* && ctx->fwd */) {
				sum = ctl_caev_sub(sum, caev);
			} else if (!ctx->fwd/* && ctx->rev */) {
				sum = ctl_caev_add(sum, caev);
			} else /*if (ctx->fwd && ctx->rev)*/ {
				sum = ctl_caev_add(caev, sum);
			}
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					PACK(struct adj_in_s,
					     .rdr = &r,
					     .adj_param.c = &sum));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				PACK(struct wrr_in_s, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
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

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	/* initialise product */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev;
			float fctr;
			float aadj;

			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			caev = *(const ctl_caev_t*)ev->msg;
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
					PACK(struct adj_in_s,
					     .rdr = &r, .adj_param.f = prod));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				PACK(struct wrr_in_s, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
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
 * this is the total return forward adjustment */
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

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);

	/* initialise another wheap and another prod */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev;
			struct fa_s cell;

			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			caev = *(const ctl_caev_t*)ev->msg;
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

		/* apply caev sum to price lines */
		do {
			/* no need to use the strtod32() reader */
			last = strtof(ln->ln, NULL);
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
	}

	/* end of first pass */
	free_coru(rdr);
	free_coru(pop);
	fini_coru();

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

	/* last pass */
	fseek(f, 0, SEEK_SET);

	init_coru();
	rdr = make_coru(co_appl_rdr, .f = f);
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	last = NAN;
	size_t i;
	for (ln = next(rdr), i = 0U; ln != NULL;) {
		/* mul up factors in between price lines */
		for (;
		     LIKELY(i < nfa) && UNLIKELY(!__inst_lt_p(ln->t, fa[i].t));
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
					PACK(struct adj_in_s,
					     .rdr = &r, .adj_param.f = prod));
				a = massage_adj(tmp);
			}

			/* off to the writer */
			next_with(
				wrr,
				PACK(struct wrr_in_s, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY((i >= nfa || __inst_lt_p(ln->t, fa[i].t))));
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


#if defined STANDALONE
#if defined __INTEL_COMPILER
# pragma warning (disable:593)
#endif	/* __INTEL_COMPILER */
#include "cattle.xh"
#include "cattle.x"
#if defined __INTEL_COMPILER
# pragma warning (default:593)
#endif	/* __INTEL_COMPILER */

static int
cmd_print(struct ctl_args_info argi[static 1U])
{
	static const char usg[] = "Usage: cattle print [CAEVs...]\n";
	static struct ctl_ctx_s ctx[1];
	int res = 0;

	if (argi->inputs_num < 1U) {
		fputs(usg, stderr);
		res = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		res = 1;
		goto out;
	}

	for (unsigned int i = 1U; i < argi->inputs_num; i++) {
		const char *fn = argi->inputs[i];

		if (UNLIKELY(ctl_read_caev_file(ctx, fn) < 0)) {
			error("cannot open file `%s'", fn);
			res = 1;
			goto out;
		}
	}

	ctl_caev_t sum = ctl_zero_caev();
	const ctl_caev_t *prev = &sum;
	for (echs_instant_t t; !__inst_0_p(t = ctl_wheap_top_rank(ctx->q));) {
		uintptr_t tmp = ctl_wheap_pop(ctx->q);
		const ctl_caev_t *this = (const void*)tmp;
		char buf[256U];
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		if (argi->unique_given && !memcmp(this, prev, sizeof(*this))) {
			/* completely identical */
			continue;
		}

		if (!argi->summary_given) {
			bp += dt_strf(bp, ep - bp, t);
			*bp++ = '\t';
			bp += ctl_caev_wr(bp, ep - bp, *this);
			*bp++ = '\n';
			*bp = '\0';
			fputs(buf, stdout);
			prev = this;
		} else {
			sum = ctl_caev_add(sum, *this);
		}
	}
	if (argi->summary_given) {
		char buf[256U];
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		bp += ctl_caev_wr(bp, ep - bp, sum);
		*bp++ = '\n';
		*bp = '\0';
		fputs(buf, stdout);
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_wheap(ctx->q);
	}
	return res;
}

static int
cmd_apply(struct ctl_args_info argi[static 1U])
{
	static const char usg[] = "Usage: cattle apply PRICES [CAEV]\n";
	static struct ctl_ctx_s ctx[1];
	int res = 0;

	if (argi->inputs_num < 2U) {
		fputs(usg, stderr);
		res = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		res = 1;
		goto out;
	}

	if (argi->reverse_given) {
		ctx->rev = 1U;
	}
	if (argi->forward_given) {
		ctx->fwd = 1U;
	}
	if (argi->precision_given) {
		const char *p = argi->precision_arg;
		char *on;

		if (*p == '+' || *p == '-') {
			;
		} else {
			ctx->abs_prec = 1U;
		}
		if ((ctx->prec = -strtol(p, &on, 10), *on)) {
			error("invalid precision `%s'", argi->precision_arg);
			res = 1;
			goto out;
		}
	}

	/* open caev file and read */
	with (const char *caev_fn =
	      argi->inputs_num > 2U ? argi->inputs[2U] : NULL) {
		if (UNLIKELY(ctl_read_caev_file(ctx, caev_fn) < 0)) {
			error("cannot open caev file `%s'", caev_fn);
			res = 1;
			goto out;
		}
	}

	/* open time series file */
	with (const char *tser_fn = argi->inputs[1U]) {
		if (argi->total_return_given && !ctx->fwd) {
			/* total return back adjustment needs 2 scans */
			if (UNLIKELY(ctl_badj_caev_file(ctx, tser_fn) < 0)) {
				error("\
cannot deduce factors for total return adjustment from `%s'", tser_fn);
				res = 1;
				goto out;
			}
		} else if (argi->total_return_given/* && ctx->fwd */) {
			if (UNLIKELY(ctl_fadj_caev_file(ctx, tser_fn) < 0)) {
				error("\
cannot deduce factors for total return adjustment from `%s'", tser_fn);
				res = 1;
				goto out;
			}
		} else if (UNLIKELY(ctl_appl_caev_file(ctx, tser_fn) < 0)) {
			error("cannot open series file `%s'", tser_fn);
			res = 1;
			goto out;
		}
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_wheap(ctx->q);
		ctx->q = NULL;
	}
	return res;
}

int
main(int argc, char *argv[])
{
	struct ctl_args_info argi[1];
	int res = 0;

	if (ctl_parser(argc, argv, argi)) {
		res = 1;
		goto out;
	} else if (argi->inputs_num < 1) {
		ctl_parser_print_help();
		res = 1;
		goto out;
	}

	/* get the coroutines going */
	init_coru_core();

	/* check the command */
	with (const char *cmd = argi->inputs[0U]) {
		if (!strcmp(cmd, "apply")) {
			res = cmd_apply(argi);
		} else if (!strcmp(cmd, "print")) {
			res = cmd_print(argi);
		} else {
			error("No command specified.\n\
See --help to obtain a list of available commands.");
			res = 1;
			goto out;
		}
	}

out:
	/* just to make sure */
	fflush(stdout);
	ctl_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* cattle.c ends here */
