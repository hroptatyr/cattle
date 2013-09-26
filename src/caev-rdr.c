/*** caev-rdr.c -- reader for caev message strings
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
#include "caev-supp.h"
#include "caev-rdr.h"
#include "ctl-dfp754.h"
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


static ctl_fld_val_t
snarf_fv(ctl_fld_key_t fc, const char *s)
{
	ctl_fld_val_t res = {};
	const char *vp = *s == '=' ? s : strchr(s, '=');

	if (UNLIKELY(vp++ == NULL)) {
		return res;
	} else if (*vp == '"' || *vp == '\'') {
		/* dequote */
		vp++;
	}

	switch (ctl_fld_type(fc)) {
	case CTL_FLD_TYPE_ADMIN:
		;
		break;

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
		res.ratio = (ctl_ratio_t){p, q};
		break;
	}
	case CTL_FLD_TYPE_PRICE: {
		char *pp;
		_Decimal32 p;

		p = strtobid32(vp, &pp);
		if (*pp != '"' && *pp != '\'') {
			break;
		}
		res.price = (ctl_price_t)p;
		break;
	}
	case CTL_FLD_TYPE_PERIO:
		;
		break;

	default:
		return res;
	}
	return res;
}


ctl_caev_t
ctl_caev_rdr(struct ctl_ctx_s *UNUSED(ctx), echs_instant_t t, const char *s)
{
	static struct ctl_fld_s *flds;
	static size_t nflds;
	ctl_caev_code_t ccod;
	ctl_caev_t res = {};
	size_t fldi;

	if (strncmp(s, "caev=", sizeof("caev=") - 1U)) {
		/* not a caev message */
		goto out;
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
			goto out;
		}
		/* otherwise assign the caev code */
		ccod = m->code;
		/* and fast forward s */
		s += 4U;
	}

#define CHECK_FLDS							\
	if (UNLIKELY(fldi >= nflds)) {					\
		const size_t nu = nflds + 64U;				\
		flds = realloc(flds, nu * sizeof(*flds));		\
		memset(flds + nflds, 0, (nu - nflds) * sizeof(*flds));	\
		nflds = nu;						\
	}

	/* reset field counter */
	fldi = 0U;
	/* add the instant passed onto us as ex-date */
	CHECK_FLDS;
	flds[fldi++] = MAKE_CTL_FLD(admin, CTL_FLD_CAEV, ccod);
	flds[fldi++] = MAKE_CTL_FLD(date, CTL_FLD_XDTE, t);
	/* now look for .XXXX= or .XXXX/ */
	for (const char *sp = s; (sp = strchr(sp, '.')) != NULL; sp++) {
		switch (sp[5U]) {
			ctl_fld_rdr_t f;
			ctl_fld_unk_t fc;
			ctl_fld_val_t fv;
		case '/':
		case '=':
			/* ah, could be a field */
			if ((f = __ctl_fldify(sp + 1U, 4U)) == NULL) {
				break;
			}
			/* otherwise we've got the code */
			fc = (ctl_fld_unk_t)f->fc;
			fv = snarf_fv(fc, sp += 5U);

			/* bang to array */
			if (fldi >= nflds) {
				const size_t nu = nflds + 64U;
				flds = realloc(flds, nu * sizeof(*flds));
				memset(flds + nflds, 0, 64U * sizeof(*flds));
				nflds = nu;
			}
			/* actually add the field now */
			flds[fldi++] = (ctl_fld_t){fc, fv};
			break;
		default:
			break;
		}
	}
	/* just let the actual mt564 support figure everything out */
	res = make_caev(flds, fldi);
out:
	return res;
}

/* caev-rdr.c ends here */
