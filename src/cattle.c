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
#include "nifty.h"
#include "dt-strpf.h"
#include "wheap.h"

struct ctl_ctx_s {
	ctl_wheap_t q;
	ctl_caev_t sum;
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
ctl_ctx_t
ctl_open_caev_file(const char *fn)
{
/* wants a const char *fn */
	static ctl_caev_t *caevs;
	static size_t ncaevs;
	char *line = NULL;
	size_t llen = 0UL;
	ssize_t nrd;
	struct ctl_ctx_s *ctx; 
	FILE *f;
	size_t caevi = 0U;

	if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		goto nul;
	} else if (UNLIKELY((ctx = calloc(1, sizeof(*ctx))) == NULL)) {
		goto nul;
	} else if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		goto nul;
	}

	while ((nrd = getline(&line, &llen, f)) > 0) {
		char *p;
		echs_instant_t t;

		if (*line == '#') {
			continue;
		} else if ((p = strchr(line, '\t')) == NULL) {
			break;
		} else if (__inst_0_p(t = dt_strp(line))) {
			break;
		}
		/* otherwise try to read the whole shebang */
		with (ctl_caev_t c = ctl_caev_rdr(ctx, t, p + 1U)) {
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
			ctl_wheap_add_deferred(ctx->q, t, qmsg);
			/* also sum them up */
			ctx->sum = ctl_caev_add(ctx->sum, c);
		}
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(ctx->q);
nul:
	if (f != NULL) {
		fclose(f);
	}
	if (line != NULL) {
		free(line);
	}
	return ctx;
}

void
ctl_close_caev_file(ctl_ctx_t x)
{
	if (LIKELY(x->q != NULL)) {
		free_ctl_wheap(x->q);
	}
	free(x);
	return;
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

#include "../test/caev-io.c"

static int
cmd_print(struct ctl_args_info argi[static 1U])
{
	static const char usg[] = "Usage: cattle print FILEs...\n";
	ctl_ctx_t ctx;
	int res = 0;

	if (argi->inputs_num < 2U) {
		fputs(usg, stderr);
		res = 1;
		goto out;
	}

	with (const char *fn = argi->inputs[1U]) {
		if (UNLIKELY((ctx = ctl_open_caev_file(fn)) == NULL)) {
			error("cannot open file `%s'", fn);
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

	/* and out again */
	ctl_close_caev_file(ctx);
out:
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

	/* check the command */
	with (const char *cmd = argi->inputs[0U]) {
		if (!strcmp(cmd, "print")) {
			res = cmd_print(argi);
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
