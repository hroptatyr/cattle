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
#include <string.h>
#include "cattle.h"
#include "caev.h"

static void
ctl_caev_pr(ctl_caev_t e)
{
	printf("m: %d<-%u + %f\t", e.mktprc.r.p, e.mktprc.r.q, (float)e.mktprc.a);
	printf("n: %d<-%u + %f\t", e.nomval.r.p, e.nomval.r.q, (float)e.nomval.a);
	printf("o: %d<-%u + %f\n", e.outsec.r.p, e.outsec.r.q, (float)e.outsec.a);
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

int
main(int argc, char *argv[])
{
	struct gengetopt_args_info argi[1];
	int res = 0;

	if (cmdline_parser(argc, argv, argi)) {
		goto out;
	}

	ctl_caev_t dvca[] = {{
			.mktprc.a = -2.00df,
		}, {
			.mktprc.a = -1.50df,
		}};
	ctl_caev_t splf[] = {{
			.mktprc.r = (ctl_ratio_t){10, 1},
		}, {
			.mktprc.r = (ctl_ratio_t){5, 2},
		}};

	ctl_caev_pr(dvca[0]);
	ctl_caev_pr(dvca[1]);
	ctl_caev_pr(ctl_caev_add(dvca[0], dvca[1]));

	ctl_caev_pr(splf[0]);
	ctl_caev_pr(splf[1]);
	ctl_caev_pr(ctl_caev_add(splf[0], splf[1]));

out:
	/* just to make sure */
	fflush(stdout);
	cmdline_parser_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* cattle.c ends here */
