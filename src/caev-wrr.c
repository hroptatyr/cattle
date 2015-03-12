/*** caev-wrr.c -- writer for caev message strings
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
#include <stdio.h>
#include <ctype.h>
#include "caev-supp.h"
#include "caev-rdr.h"
#include "caev-wrr.h"
#include "ctl-dfp754.h"
#include "dt-strpf.h"
#include "intern.h"
#include "nifty.h"

static const char xdte[] = "\"xxdt\":";

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



ssize_t
ctl_caev_wrr(char *restrict buf, size_t bsz, echs_instant_t x, ctl_caev_t c)
{
	static const char caev[] = "\"caev\":\"CTL1\"";
	static const char mkti[] = "\"xmkt\":";
	static const char nomi[] = "\"xnom\":";
	static const char outi[] = "\"xout\":";
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

	*bp++ = '{';
	bp += BANG_LIT(bp, ep, caev);
	if (!echs_nul_instant_p(x)) {
		*bp++ = ',';
		bp += BANG_LIT(bp, ep, xdte);
		*bp++ = '"';
		bp += dt_strf(bp, ep - bp, x);
		*bp++ = '"';
	}
	*bp++ = ',';
	bp += BANG_LIT(bp, ep, mkti);
	*bp++ = '"';
	bp += d32tostr(bp, ep - bp, c.mktprc.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.mktprc.r.p, c.mktprc.r.q);
	*bp++ = '"';
	*bp++ = ',';
	bp += BANG_LIT(bp, ep, nomi);
	*bp++ = '"';
	bp += d32tostr(bp, ep - bp, c.nomval.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.nomval.r.p, c.nomval.r.q);
	*bp++ = '"';
	*bp++ = ',';
	bp += BANG_LIT(bp, ep, outi);
	*bp++ = '"';
	bp += d32tostr(bp, ep - bp, c.outsec.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.outsec.r.p, c.outsec.r.q);
	*bp++ = '"';
	*bp++ = '}';
	*bp = '\0';
	return bp - buf;
}

ssize_t
ctl_kv_wrr(char *restrict buf, size_t bsz, echs_instant_t x, ctl_kvv_t flds)
{
	char *restrict bp = buf;
	const char *const ep = buf + bsz;

	*bp++ = '{';
	for (size_t i = 0U; i < flds->nkvv && bp + 4U < ep; i++) {
		struct ctl_kv_s kv = flds->kvv[i];

		if (UNLIKELY(!kv.key)) {
			/* skip this guy altogether */
			continue;
		} else if (UNLIKELY(i == 1U) && !echs_nul_instant_p(x)) {
			/* quick interim,  */
			bp += BANG_LIT(bp, ep, xdte);
			*bp++ = '"';
			bp += dt_strf(bp, ep - bp, x);
			*bp++ = '"';
			*bp++ = ',';
		}
		*bp++ = '"';
		bp += xstrlcpy(bp, obint_name(kv.key), ep - bp);
		*bp++ = '"';
		*bp++ = ':';
		*bp++ = '"';
		if (LIKELY(kv.val)) {
			bp += xstrlcpy(bp, obint_name(kv.val), ep - bp);
		}
		*bp++ = '"';
		*bp++ = ',';
	}
	if (bp[-1] == ',') {
		/* don't want no dangling comma, do we? */
		bp--;
	}
	*bp++ = '}';
	*bp = '\0';
	return bp - buf;
}

/* caev-wrr.c ends here */
