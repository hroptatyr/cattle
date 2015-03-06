/*** catest.c -- tool to count corporate actions frequencies
 *
 * Copyright (C) 2015 Sebastian Freundt
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
#include <sys/mman.h>
#include <math.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "cattle.h"
#include "caev.h"
#include "caev-rdr.h"
#include "caev-supp.h"
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

	/* use prec fractional digits if abs_prec */
	signed int prec;
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


/* public api, might go to libcattle one day */
static int
ctl_read_kv_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn */
	coru_t rdr;
	FILE *f;

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(ctl_co_rdr, f);

	for (const struct ctl_co_rdr_res_s *ln; (ln = next(rdr));) {
		/* try to read the whole shebang */
		ctl_kvv_t v = ctl_kv_rdr(ln->ln);

		/* insert to heap */
		ctl_wheap_add_deferred(ctx->q, ln->t, (colour_t){.flds = v});
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(ctx->q);
	free_coru(rdr);
	fclose(f);
	fini_coru();
	return 0;
}


/* beef */
static int
ctl_test_kv_by_caev(struct ctl_ctx_s ctx[static 1U])
{
/* test GROUP BY CAEV */
	echs_instant_t ldat[CTL_NCAEV] = {0U};
	int rc = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q));) {
		ctl_kvv_t this = ctl_wheap_pop(ctx->q).flds;
		ctl_caev_code_t ccod = ctl_kvv_get_caev_code(this);

		if (LIKELY(!echs_nul_instant_p(ldat[ccod]))) {
			echs_idiff_t d = echs_instant_diff(t, ldat[ccod]);

			printf("%s\t%d\n", caev_names[ccod], d.dd);
		}
		ldat[ccod] = t;
	}
	return rc;
}

#if 0
static int
ctl_test_kv_by_1(struct ctl_ctx_s ctx[static 1U])
{
/* test differences absolutely */
	echs_instant_t ldat = echs_nul_instant();
	int rc = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q));) {
		ctl_kvv_t this = ctl_wheap_pop(ctx->q).flds;

		if (LIKELY(!echs_nul_instant_p(ldat))) {
			echs_idiff_t d = echs_instant_diff(t, ldat);

			printf("%d\n", d.dd);
		}
		ldat = t;
	}
	return rc;
}
#endif	/* 0 */


#if defined STANDALONE
#include "catest.yucc"

int
main(int argc, char *argv[])
{
	static struct ctl_ctx_s ctx[1];
	yuck_t argi[1U];
	size_t i;
	int rc = 0;

	if (yuck_parse(argi, argc, argv)) {
		rc = 99;
		goto out;
	}

	/* get the coroutines going */
	init_coru_core();

	/* prepare the context */
	if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		goto out;
	}
	/* read all caev files */
	i = 0U;
	if (argi->nargs == 0U) {
		goto one_off;
	}
	for (; i < argi->nargs; i++) {
	one_off:
		if (UNLIKELY(ctl_read_kv_file(ctx, argi->args[i]) < 0)) {
			error("cannot open file `%s'", argi->args[i]);
			goto out;
		}
	}
	/* run the bucketiser */
	if (ctl_test_kv_by_caev(ctx) < 0) {
		rc = 1;
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_wheap(ctx->q);
	}

	/* just to make sure */
	fflush(stdout);
	yuck_free(argi);
	return rc;
}
#endif	/* STANDALONE */

/* catest.c ends here */
