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

	float prod;
	ctl_wheap_t trwh;
};

struct tser_row_s {
	echs_instant_t d;
	_Decimal32 prc;
	_Decimal32 adj;
};


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

static _Decimal32
strtokd32(const char *ln, char **on)
{
	const char *p;

	if (*on == NULL) {
		p = ln;
	} else if (**on != '\t') {
		*on = NULL;
		return 0.df;
	} else {
		p = *on + 1U;
	}
	return strtobid32(p, on);
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

static void
pr_adj(echs_instant_t d, _Decimal32 adj)
{
	pr_ei(d);
	fputc('\t', stdout);
	pr_d32(adj);
	fputc('\n', stdout);
	return;
}

static void
pr_adjq(echs_instant_t d, float adj, _Decimal32 prc)
{
/* like pr_adj() but quantise adj first */
	pr_ei(d);
	fputc('\t', stdout);
	pr_d32(quantized32(adj, prc));
	fputc('\n', stdout);
	return;
}


/* coroutines */
struct tser_ln_s {
	echs_instant_t t;
	const char *ln;
	size_t lz;
};

struct echs_msg_s {
	echs_instant_t t;
	const void *msg;
	size_t msz;
};

DEFCORU(co_appl_rdr, {
		FILE *f;
	}, void *UNUSED(arg))
{
/* coroutine for the reader of the tseries */
	char *line = NULL;
	size_t llen = 0UL;
	ssize_t nrd;

	while ((nrd = getline(&line, &llen, CORU_CLOSUR(f))) > 0) {
		static struct tser_ln_s ln[1];
		char *p;

		if (*line == '#') {
			continue;
		} else if ((p = strchr(line, '\t')) == NULL) {
			break;
		} else if (__inst_0_p(ln->t = dt_strp(line))) {
			break;
		}
		/* pack the result structure */
		ln->ln = p + 1U;
		ln->lz = nrd - (p + 1U - line);
		YIELD(ln);
	}

	free(line);
	line = NULL;
	llen = 0U;
	return 0;
}

DEFCORU(co_appl_pop, {
		ctl_wheap_t q;
	}, void *UNUSED(arg))
{
	static struct echs_msg_s ev[1];
	ctl_wheap_t q = CORU_CLOSUR(q);

	while (!__inst_0_p(ev->t = ctl_wheap_top_rank(q))) {
		/* assume it's a ctl-caev_t */
		ev->msg = (const ctl_caev_t*)ctl_wheap_pop(q);
		ev->msz = sizeof(ctl_caev_t);
		YIELD(ev);
	}
	return 0;
}


/* public api, might go to libcattle one day */
static int
ctl_read_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn */
	static ctl_caev_t *caevs;
	static size_t ncaevs;
	size_t caevi = 0U;
	struct cocore *rdr;
	struct cocore *me;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	me = PREP();
	rdr = START_PACK(co_appl_rdr, .f = f, .next = me);
	/* initialise sum to some zero */
	ctx->sum = ctl_zero_caev();

	for (const struct tser_ln_s *ln; (ln = NEXT(rdr));) {
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
	fclose(f);
	UNPREP();
	return 0;
}

static int
ctl_appl_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices */
	struct cocore *rdr;
	struct cocore *pop;
	struct cocore *me;
	ctl_caev_t sum;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	me = PREP();
	rdr = START_PACK(co_appl_rdr, .f = f, .next = me);
	pop = START_PACK(co_appl_pop, .q = ctx->q, .next = me);

	if (!ctx->fwd && !ctx->rev) {
		sum = ctx->sum;
	} else if (!ctx->rev/* && ctx->fwd */) {
		sum = ctl_zero_caev();
	} else if (!ctx->fwd/* && ctx->rev */) {
		sum = ctl_caev_inv(ctx->sum);
	} else /*if (ctx->fwd && ctx->rev)*/ {
		sum = ctl_zero_caev();
	}

	const struct echs_msg_s *ev;
	const struct tser_ln_s *ln;
	for (ln = NEXT(rdr), ev = NEXT(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = NEXT(pop)) {
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
			char *on;
			ctl_price_t prc;
			ctl_price_t adj;

			on = NULL;
			prc = strtokd32(ln->ln, &on);
			adj = ctl_caev_act_mktprc(sum, prc);
			pr_adj(ln->t, adj);
		} while (LIKELY((ln = NEXT(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
	}

	UNPREP();

	fclose(f);
	return 0;
}

static int
ctl_fctr_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices */
	struct cocore *rdr;
	struct cocore *pop;
	struct cocore *me;
	ctl_caev_t sum;
	int res = 0;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	me = PREP();
	rdr = START_PACK(co_appl_rdr, .f = f, .next = me);
	pop = START_PACK(co_appl_pop, .q = ctx->q, .next = me);

	/* initialise product */
	ctx->prod = 1.0;
	ctx->trwh = make_ctl_wheap();

	if (ctx->rev && !ctx->fwd) {
		sum = ctl_caev_inv(ctx->sum);
	} else if (ctx->rev) {
		sum = ctl_zero_caev();
	}

	ctl_price_t last = 0.df;
	const struct echs_msg_s *ev;
	const struct tser_ln_s *ln;
	for (ln = NEXT(rdr), ev = NEXT(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = NEXT(pop)) {
			ctl_caev_t caev;
			union {
				uintptr_t u;
				float x;
			} fctr;
			float lstprc = last;
			float mktprc;

			if (UNLIKELY(!last)) {
				res = -1;
				goto out;
			}

			caev = *(const ctl_caev_t*)ev->msg;
			mktprc = caev.mktprc.a;

			if (!ctx->rev) {
				/* all's good */
				;
			} else if (!ctx->fwd) {
				sum = ctl_caev_add(sum, caev);
				mktprc /= ratio_to_float(sum.mktprc.r);
				lstprc -= mktprc;
			} else {
				sum = ctl_caev_add(caev, sum);
				mktprc /= ratio_to_float(sum.mktprc.r);
			}
			fctr.x = 1.f + mktprc / lstprc;
			fctr.x *= ratio_to_float(caev.mktprc.r);
			ctx->prod *= fctr.x;

			/* push to tr wheap */
			ctl_wheap_add_deferred(ctx->trwh, ev->t, fctr.u);
		}

		/* apply caev sum to price lines */
		do {
			char *on = NULL;

			last = strtokd32(ln->ln, &on);
		} while (LIKELY((ln = NEXT(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
	}
	/* fix up the wheap */
	ctl_wheap_fix_deferred(ctx->trwh);

out:
	/* finished, yay */
	UNPREP();

	fclose(f);
	return res;
}

static int
ctl_appl_fctr_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices */
	struct cocore *rdr;
	struct cocore *pop;
	struct cocore *me;
	float sum;
	FILE *f;

	if (fn == NULL) {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	me = PREP();
	rdr = START_PACK(co_appl_rdr, .f = f, .next = me);
	pop = START_PACK(co_appl_pop, .q = ctx->trwh, .next = me);

	if (!ctx->rev && !ctx->fwd) {
		sum = ctx->prod;
	} else if (!ctx->rev/* && ctx->fwd */) {
		sum = 1.f;
	} else if (!ctx->fwd/* && ctx->rev */) {
		sum = 1.f / ctx->prod;
	} else /*if (ctx->rev && ctx->fwd)*/ {
		sum = 1.f;
	}

	const struct echs_msg_s *ev;
	const struct tser_ln_s *ln;
	for (ln = NEXT(rdr), ev = NEXT(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) && UNLIKELY(!__inst_lt_p(ln->t, ev->t));
		     ev = NEXT(pop)) {
			union {
				uintptr_t u;
				float x;
			} fctr;

			fctr.u = (uintptr_t)ev->msg;

			/* compute the new sum */
			if (!ctx->rev) {
				sum /= fctr.x;
			} else {
				sum *= fctr.x;
			}
		}

		/* apply caev sum to price lines */
		do {
			char *on;
			ctl_price_t prc;
			float adj;

			on = NULL;
			prc = strtokd32(ln->ln, &on);
			adj = (float)prc * sum;
			pr_adjq(ln->t, adj, prc);
		} while (LIKELY((ln = NEXT(rdr)) != NULL) &&
			 LIKELY((ev == NULL || __inst_lt_p(ln->t, ev->t))));
	}

	UNPREP();

	fclose(f);
	return 0;
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
		const ctl_caev_t *this = (const void*)ctl_wheap_pop(ctx->q);
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

	/* open caev file and read */
	with (const char *caev_fn = argi->inputs[2U]) {
		if (UNLIKELY(ctl_read_caev_file(ctx, caev_fn) < 0)) {
			error("cannot open caev file `%s'", caev_fn);
			res = 1;
			goto out;
		}
	}

	/* open time series file */
	with (const char *tser_fn = argi->inputs[1U]) {
		if (argi->total_return_given) {
			if (UNLIKELY(ctl_fctr_caev_file(ctx, tser_fn) < 0)) {
				error("\
cannot deduce factors for total return adjustment from `%s", tser_fn);
				res = 1;
				goto out;
			}
			if (UNLIKELY(ctl_appl_fctr_file(ctx, tser_fn) < 0)) {
				error("\
cannot adjust prices from `%s", tser_fn);
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
	if (LIKELY(ctx->trwh != NULL)) {
		free_ctl_wheap(ctx->trwh);
		ctx->trwh = NULL;
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
	initialise_cocore();

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
