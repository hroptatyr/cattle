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
#include "caev-wrr.h"
#include "caev-supp.h"
#include "dfp754_d32.h"
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

static size_t
xstrlcpy(char *restrict dst, const char *src, size_t dsz)
{
	size_t ssz = strlen(src);
	if (ssz > dsz) {
		ssz = dsz - 1U;
	}
	memcpy(dst, src, ssz);
	dst[ssz] = '\0';
	return ssz;
}


/* beef */
static int
ctl_test_kv(struct ctl_ctx_s ctx[static 1U])
{
/* GROUP BY CAEV, frequency 1/d */
	echs_instant_t ldat[CTL_NCAEV] = {0U};
	int rc = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q));) {
		ctl_kvv_t this = ctl_wheap_pop(ctx->q);
		ctl_caev_code_t ccod = ctl_kvv_get_caev_code(this);
		char *bp = pr_buf;
		const char *const ep = pr_buf + sizeof(pr_buf);
		echs_idiff_t d;

		bp += xstrlcpy(bp, caev_names[ccod], ep - bp);
		*bp++ = '\t';
		if (LIKELY(!echs_nul_instant_p(ldat[ccod]))) {
			d = echs_instant_diff(t, ldat[ccod]);
			bp += snprintf(bp, ep - bp, "%d", d.dd);
		}
		*bp++ = '\t';
		bp += ctl_kv_wrr(bp, ep - bp, t, this);
		*bp = '\0';
		puts(pr_buf);

		ldat[ccod] = t;

		ctl_free_kvv(this);
	}
	return rc;
}

static int
ctl_test_kv_freq(struct ctl_ctx_s ctx[static 1U], unsigned int f)
{
/* GROUP BY CAEV frequency f/d */
	ctl_caevs_t qf = make_ctl_wheap();
	echs_instant_t ldat[CTL_NCAEV] = {0U};
	int rc = 0;

	if (UNLIKELY(qf == NULL)) {
		return -1;
	}
	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q));) {
		ctl_kvv_t this = ctl_wheap_pop(ctx->q);
		ctl_caev_code_t ccod = ctl_kvv_get_caev_code(this);

		if (LIKELY(!echs_nul_instant_p(ldat[ccod]))) {
			echs_idiff_t d = echs_instant_diff(t, ldat[ccod]);
			echs_instant_t per = {
				.dpart = (d.dd + f - 1U) / f,
				/* lest we end up with the nul instance */
				.intra = 1U,
			};

			ctl_wheap_add_deferred(qf, per, (colour_t)this);
		}
		ldat[ccod] = t;
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(qf);

	for (echs_instant_t per;
	     !echs_nul_instant_p(per = ctl_wheap_top_rank(qf));) {
		ctl_kvv_t this = ctl_wheap_pop(qf);
		ctl_caev_code_t ccod = ctl_kvv_get_caev_code(this);
		char *bp = pr_buf;
		const char *const ep = pr_buf + sizeof(pr_buf);

		bp += xstrlcpy(bp, caev_names[ccod], ep - bp);
		*bp++ = '\t';
		bp += snprintf(bp, ep - bp, "%dd", (signed int)per.dpart);
		*bp++ = '\t';
		bp += ctl_kv_wrr(bp, ep - bp, echs_nul_instant(), this);
		*bp = '\0';
		puts(pr_buf);

		ctl_free_kvv(this);
	}

	free_ctl_wheap(qf);
	return rc;
}


#if defined STANDALONE
#include "catest.yucc"

static int
cmd_stat(struct ctl_ctx_s ctx[static 1U], const struct yuck_cmd_stat_s argi[static 1U])
{
	int rc = 0;

	if (argi->freq_arg) {
		char *on;
		long unsigned int f = strtoul(argi->freq_arg, &on, 10);

		if (UNLIKELY(!f)) {
			rc = 1;
		} else if (ctl_test_kv_freq(ctx, (unsigned int)f) < 0) {
			rc = 1;
		}
	} else {
		if (ctl_test_kv(ctx) < 0) {
			rc = 1;
		}
	}
	return rc;
}

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
		if (UNLIKELY(ctl_read_caevs(ctx->q, argi->args[i]) < 0)) {
			error("cannot open file `%s'", argi->args[i]);
			goto out;
		}
	}
	/* run the bucketiser */
	switch (argi->cmd) {
	case CATEST_CMD_STAT:
		rc = cmd_stat(ctx, (const void*)argi);
		break;
	default:
		break;
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
