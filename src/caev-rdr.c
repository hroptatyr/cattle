/*** caev-rdr.h -- reader for caev message strings
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
#include <string.h>
#include <dfp754.h>
#include "caev-supp.h"
#include "caev-rdr.h"
#include "nifty.h"

/* gperf gen'd goodness */
#if defined __INTEL_COMPILER
# pragma warning (disable:869)
#endif	/* __INTEL_COMPILER */
#include "caev-rdr-gp.c"
#include "caev-fld-gp.c"
#if defined __INTEL_COMPILER
# pragma warning (default:869)
#endif	/* __INTEL_COMPILER */

typedef union {
	ctl_ratio_t rt;
	ctl_price_t pr;
	ctl_perio_t pe;
	ctl_quant_t qu;
	ctl_date_t dt;
} fldval_t;


static fldval_t
snarf_fv(ctl_fld_unk_t fc, const char *s)
{
	fldval_t res = {};
	const char *vp = *s == '=' ? s : strchr(s, '=');

	if (UNLIKELY(vp++ == NULL)) {
		return res;
	} else if (*vp == '"' || *vp == '\'') {
		/* dequote */
		vp++;
	}

	switch (ctl_fld_type(fc)) {
	case CTL_FLD_TYPE_DATE:
		;
		break;

	case CTL_FLD_TYPE_RATIO: {
		char *pp;
		signed int p;
		unsigned int q;

		p = strtol(vp, &pp, 10);
		if (*pp != ':' && *pp != '/') {
			break;
		} else if (!(q = strtoul(pp + 1U, &pp, 10))) {
			break;
		} else if (*pp != '"' && *pp != '\'') {
			break;
		}
		/* otherwise ass */
		res.rt = (ctl_ratio_t){p, q};
		break;
	}
	case CTL_FLD_TYPE_PRICE: {
		char *pp;
		_Decimal32 p;

		p = strtod32(vp, &pp);
		if (*pp != '"' && *pp != '\'') {
			break;
		}
		res.pr = (ctl_price_t)p;
		break;
	}
	case CTL_FLD_TYPE_PERIOD:
		;
		break;

	default:
		return res;
	}
	return res;
}


void
ctl_caev_rdr(struct ctl_ctx_s ctx[static 1], echs_instant_t t, const char *s)
{
	ctl_caev_code_t ccod;

	if (strncmp(s, "caev=", sizeof("caev=") - 1U)) {
		/* not a caev message */
		return;
	}
	/* otherwise try and read the code */
	switch (*(s += sizeof("caev=") - 1U)) {
	case '"':
	case '\'':
		s++;
		break;
	default:
		/* no quotes :/ wish me luck */
		break;
	}
	with (ctl_caev_rdr_t m = __ctl_caev_codify(s, 4U)) {
		if (LIKELY(m == NULL)) {
			/* not a caev message */
			return;
		}
		/* otherwise assign the caev code */
		ccod = m->code;
		/* and fast forward s */
		s += 4U;
	}

	/* now look for .XXXX= or .XXXX/ */
	for (const char *sp = s; (sp = strchr(sp, '.')) != NULL; sp++) {
		switch (sp[5U]) {
			ctl_fld_rdr_t f;
			ctl_fld_unk_t fc;
			fldval_t fv;
		case '/':
		case '=':
			/* ah, could be a field */
			if ((f = __ctl_fldify(sp + 1U, 4U)) == NULL) {
				break;
			}
			/* otherwise we've got the code */
			fc = (ctl_fld_unk_t)f->fc;
			fv = snarf_fv(fc, sp += 5U);

			switch (ctl_fld_type(fc)) {
			case CTL_FLD_TYPE_PRICE:
				printf("got %u -> %f\n", fc, (float)fv.pr);
				break;
			case CTL_FLD_TYPE_RATIO:
				printf("got %u -> %i:%u\n", fc, fv.rt.p, fv.rt.q);
				break;
			}
			break;
		default:
			break;
		}
	}
	return;
}

/* caev-rdr.c ends here */
