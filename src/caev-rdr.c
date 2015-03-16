/*** caev-rdr.c -- reader for caev message strings
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
#include <string.h>
#include <ctype.h>
#include "jsmn.h"
#include "caev-supp.h"
#include "caev-rdr.h"
#include "ctl-dfp754.h"
#include "intern.h"
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

		p = strtol(s, &pp, 10);
		if (*pp != ':' && *pp != '/') {
			break;
		} else if (!(q = strtoul(pp + 1U, &pp, 10))) {
			break;
		}
		/* otherwise ass */
		res.ratio = (ctl_ratio_t){p, q};
		break;
	}
	case CTL_FLD_TYPE_PRICE: {
		char *pp;
		_Decimal32 p;

		p = strtod32(s, &pp);
		res.price = (ctl_price_t)p;
		break;
	}
	case CTL_FLD_TYPE_PERIO:
		;
		break;

	case CTL_FLD_TYPE_CUSTM: {
		char *pp;
		_Decimal32 v;
		signed int p;
		unsigned int q;

		v = strtod32(s, &pp);
		if (*pp != '+' && *pp != '-') {
			break;
		}
		p = strtol(pp, &pp, 10);
		if (pp[0] != '<' || pp[1] != '-') {
			break;
		}
		q = strtoul(pp + 2U, &pp, 10),
		/* otherwise ass */
		res.custm = (ctl_custm_t){.r = (ctl_ratio_t){p, q}, .a = v};
		break;
	}
	default:
		return res;
	}
	return res;
}

static ctl_kvv_t
make_kvv(const struct ctl_kv_s *f, size_t n)
{
	ctl_kvv_t res = malloc(sizeof(*res) + n * sizeof(*res->kvv));

	memcpy(res->kvv, f, (res->nkvv = n) * sizeof(*res->kvv));
	return res;
}


/* readers, all of which write into FLDS */
static struct ctl_kv_s *kvs;
static size_t nkvs;

static obint_t caev;
static obint_t xxdt;
static obint_t xdte;
static obint_t effd;

#define CHECK_FLDS(x, i, z)						\
	if (UNLIKELY((i) >= (z))) {					\
		const size_t nu = (z) + 64U;				\
		x = realloc(x, nu * sizeof(*x));			\
		memset(x + (z), 0, (nu - (z)) * sizeof(*x));		\
		z = nu;							\
	}

#define CHECK_XDT				\
	if (UNLIKELY(!caev)) {			\
		caev = intern("caev", 4U);	\
		xxdt = intern("xxdt", 4U);	\
		xdte = intern("xdte", 4U);	\
		effd = intern("effd", 4U);	\
	}

static ctl_kvv_t
_read_ctlold(const char *s, size_t z)
{
/* ye olde key="value" reader */
	const char *const ep = s + z;
	const char *cp;
	size_t fldi = 0U;
	size_t caevi = 0U;
	size_t xxdti = 0U;
	obint_t xdt = 0U;

	CHECK_XDT;
	do {
		char dlm = ' ';

		/* fast forward to a field */
		for (; s < ep && !isalpha(*s); s++);
		if (UNLIKELY(s >= ep)) {
			break;
		}

		/* fast forward to the assignment */
		for (cp = s; s < ep && *s != '='; s++);
		if (UNLIKELY(s >= ep)) {
			break;
		}

		/* use CHECK_FLDS from above */
		CHECK_FLDS(kvs, fldi, nkvs);

		/* get interned field */
		kvs[fldi].key = intern(cp, s++ - cp);
		if (UNLIKELY(s >= ep)) {
			break;
		}

		/* otherwise try and read the code */
		switch (*s) {
		case '"':
		case '\'':
			dlm = *s;
			s++;
			break;
		default:
			/* no quotes :/ wish me luck */
			break;
		}
		for (cp = s; s < ep && *s >= ' ' && *s != dlm; s++);
		if (UNLIKELY(s >= ep)) {
			break;
		}
		/* get interned value */
		kvs[fldi].val = intern(cp, s - cp);

		/* check if it's a xxdt contributing field */
		if (kvs[fldi].key == xdte || kvs[fldi].key == effd) {
			if (!xxdti) {
				xdt = kvs[fldi].val;
			}
		} else if (kvs[fldi].key == xxdt) {
			/* always track this one */
			xxdti = fldi + 1U;
		} else if (kvs[fldi].key == caev) {
			caevi = fldi + 1U;
		}

		/* advance field counter */
		fldi++;
	} while (1);
	/* enrich with xxdt? */
	if (!xxdti && xdt) {
		CHECK_FLDS(kvs, fldi, nkvs);

		kvs[fldi].key = xxdt;
		kvs[fldi].val = xdt;
		xxdti = ++fldi;
	}
	/* make sure caev is 0th and xxdt is 1st */
	if (caevi > 1U) {
		struct ctl_kv_s c = kvs[caevi - 1U];
		memmove(kvs + 1U, kvs, (caevi - 1U) * sizeof(*kvs));
		kvs[0U] = c;
		if (xxdti < caevi) {
			/* it's now on pos+1 */
			xxdti++;
		}
	}
	if (xxdti && xxdti != 2U) {
		struct ctl_kv_s x = kvs[xxdti - 1U];
		memmove(kvs + 2U, kvs + 1U, (xxdti - 1U) * sizeof(*kvs));
		kvs[1U] = x;
	}
	return make_kvv(kvs, fldi);
}

static ctl_kvv_t
_read_json(const char *s, size_t z)
{
/* teh n3w json r33dr */
	size_t fldi = 0U;
	jsmn_parser p;
	jsmntok_t tok[64U];
	size_t caevi = 0U;
	size_t xxdti = 0U;
	obint_t xdt = 0U;
	int r;

	/* it's a given that we start off with { */
	if (UNLIKELY(*s != '{' && s[z - 1U] != '}')) {
		/* not json it's not */
		return NULL;
	}

	CHECK_XDT;

	jsmn_init(&p);
	if ((r = jsmn_parse(&p, s, z, tok, countof(tok))) < 0) {
		/* didn't work */
		return NULL;
	}

	/* top-level element should be an object */
	if (r < 1 || tok->type != JSMN_OBJECT) {
		return NULL;
	}

	/* loop and intern */
	for (int i = 1; i < r; i++) {
		const char *ks;
		size_t kz;

		CHECK_FLDS(kvs, fldi, nkvs);
		ks = s + tok[i].start;
		kz = tok[i].end - tok[i].start;
		kvs[fldi].key = intern(ks, kz);
		i++;
		ks = s + tok[i].start;
		kz = tok[i].end - tok[i].start;
		kvs[fldi].val = intern(ks, kz);

		/* check if it's a xxdt contributing field */
		if (kvs[fldi].key == xdte || kvs[fldi].key == effd) {
			if (!xxdti) {
				xdt = kvs[fldi].val;
			}
		} else if (kvs[fldi].key == xxdt) {
			/* always track this one */
			xxdti = fldi + 1U;
		} else if (kvs[fldi].key == caev) {
			caevi = fldi + 1U;
		}

		/* advance field index */
		fldi++;
	}
	/* enrich with xxdt? */
	if (!xxdti && xdt) {
		CHECK_FLDS(kvs, fldi, nkvs);

		kvs[fldi].key = xxdt;
		kvs[fldi].val = xdt;
		xxdti = ++fldi;
	}
	/* make sure caev is 0th and xxdt is 1st */
	if (caevi > 1U) {
		struct ctl_kv_s c = kvs[caevi - 1U];
		memmove(kvs + 1U, kvs, (caevi - 1U) * sizeof(*kvs));
		kvs[0U] = c;
		if (xxdti < caevi) {
			/* it's now on pos+1 */
			xxdti++;
		}
	}
	if (xxdti && xxdti != 2U) {
		struct ctl_kv_s x = kvs[xxdti - 1U];
		memmove(kvs + 2U, kvs + 1U, (xxdti - 1U) * sizeof(*kvs));
		kvs[1U] = x;
	}
	return make_kvv(kvs, fldi);
}


__attribute__((pure, const)) ctl_caev_code_t
ctl_kvv_get_caev_code(ctl_kvv_t fldv)
{
	if (UNLIKELY(fldv->nkvv == 0U)) {
		goto out;
	}
	with (ctl_fld_rdr_t f) {
		/* assume first field is the caev indicator */
		const char *k0 = obint_name(fldv->kvv[0].key);

		if ((f = __ctl_fldify(k0, 4U)) == NULL) {
			/* not a caev message */
			goto out;
		} else if ((ctl_fld_admin_t)f->fc != CTL_FLD_CAEV) {
			/* a field but not a caev message */
			goto out;
		}
	}
	/* otherwise try and read the code */
	with (ctl_caev_rdr_t m) {
		const char *v0 = obint_name(fldv->kvv[0].val);

		if (UNLIKELY((m = __ctl_caev_codify(v0, 4U)) == NULL)) {
			/* not a caev message */
			goto out;
		}
		/* otherwise return the caev code */
		return m->code;
	}
out:
	return CTL_CAEV_UNK;
}

ctl_caev_t
ctl_kvv_get_caev(ctl_kvv_t fldv)
{
	static struct ctl_fld_s *flds;
	static size_t nflds;
	ctl_caev_code_t ccod;
	ctl_caev_t res = ctl_zero_caev();
	size_t fldi;

	if (UNLIKELY((ccod = ctl_kvv_get_caev_code(fldv)) == CTL_CAEV_UNK)) {
		/* not a caev message, at least not one we support */
		goto out;
	}

	/* reset field counter */
	fldi = 0U;
	/* add the instant passed onto us as ex-date */
	CHECK_FLDS(flds, fldi, nflds);
	flds[fldi++] = MAKE_CTL_FLD(admin, CTL_FLD_CAEV, ccod);
	/* go through all them fields then */
	for (size_t i = 1U; i < fldv->nkvv; i++) {
		const char *k = obint_name(fldv->kvv[i].key);
		const char *v = obint_name(fldv->kvv[i].val);
		ctl_fld_rdr_t f;
		ctl_fld_unk_t fc;
		ctl_fld_val_t fv;

		if ((f = __ctl_fldify(k, 4U)) == NULL) {
			/* not a field then aye */
			continue;
		}
		/* otherwise we've got the code */
		fc = (ctl_fld_unk_t)f->fc;
		fv = snarf_fv(fc, v);

		/* bang to array */
		CHECK_FLDS(flds, fldi, nflds);
		/* actually add the field now */
		flds[fldi++] = (ctl_fld_t){{fc}, fv};
	}
	/* just let the actual mt564 support figure everything out */
	res = make_caev(flds, fldi);
out:
	return res;
}

ctl_kvv_t
ctl_kv_rdr(const char *msg, size_t len)
{
	if (UNLIKELY(len == 0U)) {
		return NULL;
	} else if (LIKELY(*msg == '{')) {
		/* json! */
		return _read_json(msg, len);
	}
	/* otherwise try that old key=value thing */
	return _read_ctlold(msg, len);
}

void
ctl_free_kvv(ctl_kvv_t f)
{
	free(f);
	return;
}

/* caev-rdr.c ends here */
