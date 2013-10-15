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
};

#include "../test/caev-io.c"


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
	const struct echs_msg_s *ev;
	enum {
		UNK,
		/* we've got lines from the rdr coru and events from pop */
		LN_ACT,
		/* we've only got lines from the rdr coru */
		LN_LIT,
	} state = UNK;
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

	if (!ctx->rev) {
		sum = ctx->sum;
	} else {
		sum = ctl_caev_rev(ctx->sum);
	}

	if ((ev = NEXT(pop)) != NULL) {
		state = LN_ACT;
	} else {
		state = LN_LIT;
	}
	switch (state) {
		const struct tser_ln_s *ln;
	default:
	case UNK:
		break;
	case LN_ACT:
		while ((ln = NEXT(rdr)) != NULL) {
			char *on = NULL;
			ctl_fund_t p;

			/* otherwise check that T is older than top of wheap */
			while (UNLIKELY(!__inst_lt_p(ln->t, ev->t))) {
				/* compute the new sum */
				ctl_caev_t caev = *(const ctl_caev_t*)ev->msg;

				if (!ctx->rev) {
					sum = ctl_caev_sub(sum, caev);
				} else {
					sum = ctl_caev_add(sum, caev);
				}
				if ((ev = NEXT(pop)) == NULL) {
					state = LN_LIT;
					goto raw;
				}
			}

			/* otherwise apply */
			pr_ei(ln->t);
			for (; (p.mktprc = strtokd32(ln->ln, &on), on);) {
				fputc('\t', stdout);
				p = ctl_caev_act(sum, p);
				pr_d32(p.mktprc);
			} while ((p.mktprc = strtokd32(ln->ln, &on), on));
			fputc('\n', stdout);
		}
		break;
	case LN_LIT:
		while ((ln = NEXT(rdr)) != NULL) {
			char *on;
			ctl_price_t p;

		raw:
			on = NULL;
			pr_ei(ln->t);
			for (; (p = strtokd32(ln->ln, &on), on);) {
				fputc('\t', stdout);
				pr_d32(p);
			} while ((p = strtokd32(ln->ln, &on), on));
			fputc('\n', stdout);
		}
		break;
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
	/* initialise sum to some zero */
	ctx->sum = ctl_zero_caev();

	for (unsigned int i = 1U; i < argi->inputs_num; i++) {
		const char *fn = argi->inputs[i];

		if (UNLIKELY(ctl_read_caev_file(ctx, fn) < 0)) {
			error("cannot open file `%s'", fn);
			res = 1;
			goto out;
		}
	}

	for (echs_instant_t t; !__inst_0_p(t = ctl_wheap_top_rank(ctx->q));) {
		uintptr_t x = ctl_wheap_pop(ctx->q);
		char buf[256U];
		char *bp = buf;

		bp += dt_strf(buf, sizeof(buf), t);
		*bp++ = '\t';
		*bp++ = '\0';
		fputs(buf, stdout);
		with (ctl_caev_t c = *(ctl_caev_t*)x) {
			ctl_caev_pr(c);
		}
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
	/* initialise sum to some zero */
	ctx->sum = ctl_zero_caev();

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
		if (UNLIKELY(ctl_appl_caev_file(ctx, tser_fn) < 0)) {
			error("cannot open series file `%s'", tser_fn);
			res = 1;
			goto out;
		}
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_wheap(ctx->q);
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
