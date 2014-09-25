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
#include <assert.h>
#include <sys/mman.h>
#include <math.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "cattle.h"
#include "caev.h"
#include "caev-rdr.h"
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
	return fwrite(bf, sizeof(*bf), d32tostr(bf, sizeof(bf), x), stdout);
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
	bp += d32tostr(bp, ep - bp, c.mktprc.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.mktprc.r.p, c.mktprc.r.q);
	*bp++ = '"';
	*bp++ = ' ';
	bp += BANG_LIT(bp, ep, nomi);
	*bp++ = '"';
	bp += d32tostr(bp, ep - bp, c.nomval.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.nomval.r.p, c.nomval.r.q);
	*bp++ = '"';
	*bp++ = ' ';
	bp += BANG_LIT(bp, ep, outi);
	*bp++ = '"';
	bp += d32tostr(bp, ep - bp, c.outsec.a);
	bp += snprintf(bp, ep - bp, "%+d<-%u", c.outsec.r.p, c.outsec.r.q);
	*bp++ = '"';
	*bp++ = '}';
	*bp = '\0';
	return bp - buf;
}

static size_t
ctl_kvv_wr(char *restrict buf, size_t bsz, ctl_kvv_t flds)
{
	char *restrict bp = buf;
	const char *const ep = buf + bsz;

	for (size_t i = 0U; i < flds->nkvv && bp + 4U < ep; i++) {
		bp += xstrlcpy(bp, obint_name(flds->kvv[i].key), ep - bp);
		*bp++ = '=';
		*bp++ = '"';
		bp += xstrlcpy(bp, obint_name(flds->kvv[i].val), ep - bp);
		*bp++ = '"';
		*bp++ = ' ';
	}
	if (bp[-1] == ' ') {
		/* don't want no dangling space, do we? */
		bp--;
	}
	*bp = '\0';
	return bp - buf;
}

static _Decimal32
mkscal(signed int nd)
{
/* produce a d32 with -ND fractional digits */
	return scalbnd32(1.df, nd);
}


/* membuf guts */
struct membuf_s {
	char *buf;
	size_t bsz;
	size_t bof;
	size_t max;
	int wfd;
	int rfd;
};

static const char*
mb_tmpnam(const struct membuf_s *mb)
{
	static char tmpnam[64U];

	/* generate a temporary file name */
	snprintf(tmpnam, sizeof(tmpnam), "/tmp/ctl_%p", mb->buf);
	return tmpnam;
}

static int
init_mb(struct membuf_s *restrict mb, size_t iniz, size_t maxz)
{
	const int prot = PROT_READ | PROT_WRITE;
	const int mapf = MAP_ANON | MAP_PRIVATE;
	const size_t pgsz = sysconf(_SC_PAGESIZE);

	if (UNLIKELY(iniz > maxz)) {
		if (LIKELY(maxz > 0U)) {
			iniz = maxz;
		}
	}
	/* now round to multiples of pgsz */
	iniz = (iniz / pgsz) * pgsz;
	maxz = (maxz / pgsz) * pgsz;

	mb->buf = mmap(NULL, (mb->bsz = iniz), prot, mapf, -1, 0);
	if (UNLIKELY(mb->buf == MAP_FAILED)) {
		return -1;
	}
	/* initialise the rest of the crew */
	mb->bof = 0U;
	mb->max = maxz;
	mb->rfd = mb->wfd = -1;
	return 0;
}

static int
free_mb(struct membuf_s *restrict mb)
{
	int rc = 0;

	if (UNLIKELY(mb->buf == NULL)) {
		rc--;
		goto wipe;
	}
	/* close the temp file, if any */
	if (UNLIKELY(mb->rfd >= 0)) {
		rc += close(mb->rfd);
	}
	if (UNLIKELY(mb->wfd >= 0)) {
		rc += close(mb->wfd);
	}
	if (UNLIKELY(mb->buf == MAP_FAILED)) {
		rc--;
		goto wipe;
	}
	rc += munmap(mb->buf, mb->bsz);
wipe:
	memset(mb, 0, sizeof(*mb));
	mb->rfd = mb->wfd = -1;
	return rc;
}

static ssize_t
mb_load(struct membuf_s *restrict mb)
{
	ssize_t nrd;

	if (mb->rfd < 0) {
		return -1;
	}
	/* memmove remainder */
	if (mb->bof && mb->bof < mb->bsz) {
		memmove(mb->buf, mb->buf + mb->bof, mb->bsz - mb->bof);
		mb->bof = mb->bsz - mb->bof;
	}
	/* read as many as mb->bsz bytes */
	if ((nrd = read(mb->rfd, mb->buf + mb->bof, mb->bsz - mb->bof)) <= 0) {
		close(mb->rfd);
		mb->rfd = -1;
		nrd = -1;
	} else {
		mb->bof += (size_t)nrd;
	}
	return nrd;
}

static ssize_t
mb_flsh(struct membuf_s *restrict mb)
{
	ssize_t tot = 0;

	if (mb->wfd < 0) {
		const int ofl = O_WRONLY | O_CREAT | O_TRUNC;
		const char *fn = mb_tmpnam(mb);

		if ((mb->wfd = open(fn, ofl, 0644)) < 0) {
			return -1;
		} else if ((mb->rfd = open(fn, O_RDONLY)) < 0) {
			return -1;
		}
		/* otherwise just get rid of the file right away */
		(void)unlink(fn);
	}
	/* now write mb->bof bytes there */
	for (ssize_t nwr;
	     (nwr = write(mb->wfd, mb->buf + tot, mb->bof - tot)) > 0;
	     tot += nwr);

	if (UNLIKELY((size_t)tot < mb->bof)) {
		/* couldn't completely write the file,
		 * do some cleaning up then */
		close(mb->wfd);
		mb->wfd = -1;
		tot = -1;
	}
	/* reset buffer offset */
	mb->bof = 0U;
	return tot;
}

static int
mb_cat(struct membuf_s *restrict mb, const char *s, size_t z)
{
	const int prot = PROT_READ | PROT_WRITE;
	const int mapf = MAP_ANON | MAP_PRIVATE;

	if (mb->bof + z > mb->max) {
	flush:
		/* flush to disk */
		if (mb_flsh(mb) < 0) {
			return -1;
		}

	} else if (mb->bof + z > mb->bsz) {
		char *const old = mb->buf;
		const size_t olz = mb->bsz;

		/* calc new size */
		for (mb->bsz *= 2U; mb->bof + z > mb->bsz; mb->bsz *= 2U);
		/* check if we need to resort to the disk */
		if (UNLIKELY(mb->bsz > mb->max)) {
			/* yep */
			mb->bsz = mb->max;
		}
		mb->buf = mmap(NULL, mb->bsz, prot, mapf, -1, 0);
		if (UNLIKELY(mb->buf == MAP_FAILED)) {
			/* we've got the old guy, use that as max value */
			mb->buf = old;
			mb->bsz = mb->max = olz;
			goto flush;
		}
		/* otherwise move the contents to the new space */
		memmove(mb->buf, old, mb->bof);
		/* and release the old space */
		munmap(old, olz);

		if (mb->bsz == mb->max) {
			goto flush;
		}
	}
	memcpy(mb->buf + mb->bof, s, z);
	mb->bof += z/*including \n*/;
	return 0;
}


/* coroutines */
struct echs_fund_s {
	echs_instant_t t;
	size_t nf;
	_Decimal32 f[3U];
};

declcoru(co_appl_rdr, {
		FILE *f;
		/* after EOF rewind to the beginning and start over */
		size_t loop;
	}, {});

static const struct rdr_res_s {
	echs_instant_t t;
	const char *ln;
	size_t lz;
} *defcoru(co_appl_rdr, c, UNUSED(arg))
{
/* coroutine for the reader of the tseries */
	char *line = NULL;
	size_t llen = 0UL;
	/* we'll yield a rdr_res */
	struct rdr_res_s res;
	/* for non-seekables we keep a buffer here */
	struct membuf_s mb = {NULL};
	/* number of times to loop over the input */
	size_t loop = c->loop;

	if (loop && fseek(c->f, 0, SEEK_SET) < 0) {
		/* we need a buffer then */
		size_t max;
		struct rlimit r;

		/* firstly determine the maximum amount of memory to use */
		if (getrlimit(RLIMIT_AS, &r) < 0) {
			max = RLIM_INFINITY;
		} else {
			max = r.rlim_cur;
		}

		init_mb(&mb, 65536U, max / 4U);
	}

rewind:
#if defined HAVE_GETLINE
	for (ssize_t nrd; (nrd = getline(&line, &llen, c->f)) > 0;)
#elif defined HAVE_FGETLN
	while ((line = fgetln(c->f, &llen)) != NULL)
#endif	/* HAVE_GETLINE || HAVE_FGETLN */
	{
		char *p;

		if (*line == '#') {
			continue;
		} else if (echs_nul_instant_p(res.t = dt_strp(line, &p))) {
			continue;
		} else if (*p != '\t') {
			continue;
		}
		/* \nul out the line */
		line[nrd - 1] = '\0';
		/* check if this guy needs buffering */
		if (mb.buf != NULL && mb_cat(&mb, line, nrd) < 0) {
			/* big bugger */
			goto out;
		}
		/* pack the result structure */
		res.ln = p + 1U;
		res.lz = nrd - 1/*\n*/ - (p + 1U - line);
		yield(res);
	}

	if (mb.buf != NULL && mb.bsz == mb.max) {
		/* dump the rest of the buffer to the disk as well */
		mb_flsh(&mb);
	}

	/* check if we ought to loop */
	while (loop--) {
		/* first of all, let everyone know about EOF */
		yield_ptr(NULL);

		if (mb.buf == NULL) {
			/* just go back to the ordinary loop innit */
			rewind(c->f);
			goto rewind;
		}

		/* load from disk */
		do {
			/* otherwise serve from buffer */
			for (const char *bp = mb.buf,
				     *const ep = mb.buf + mb.bof;
			     bp < ep; bp++) {
				const char *eol;
				char *p;

				if ((eol = memchr(bp, '\0', ep - bp)) == NULL) {
					/* let them know we've only read
					 * half a line */
					mb.bof = bp - mb.buf;
					break;
				}
				res.t = dt_strp(bp, &p);
				res.ln = p + 1U;
				res.lz = eol - p - 1U;
				/* increment by line length */
				bp = eol;
				/* and yield */
				yield(res);
			}
		} while (mb_load(&mb) >= 0);
	}

out:
	if (mb.buf != NULL) {
		free_mb(&mb);
	}

	free(line);
	line = NULL;
	llen = 0U;
	return NULL;
}

static struct echs_fund_s
massage_rdr(const struct rdr_res_s *msg)
{
/* massage a message from the rdr coru into something more useful */
	struct echs_fund_s res;
	const char *p = msg->ln - 1U;

	res.t = msg->t;
	res.nf = 0U;
	for (size_t i = 0U; i < countof(res.f) && *p; i++) {
		char *next;
		res.f[res.nf++] = strtod32(p + 1U, &next);
		p = next;
	}
	return res;
}

declcoru(co_appl_pop, {
		ctl_caevs_t q;
	}, {});

static const struct pop_res_s {
	echs_instant_t t;
	const void *msg;
	size_t msz;
} *defcoru(co_appl_pop, c, UNUSED(arg))
{
	/* we'll yield a pop_res_s */
	static ctl_caev_t this[1U];
	struct pop_res_s res;

	while (!echs_nul_instant_p(res.t = ctl_wheap_top_rank(c->q))) {
		/* turn all date stamps into full date/time stamps */
		if (echs_instant_all_day_p(res.t)) {
			res.t.H = 0U;
			res.t.M = 0U;
			res.t.S = 0U;
			res.t.ms = 0U;
		} else if (echs_instant_all_sec_p(res.t)) {
			res.t.ms = 0U;
		}
		/* assume it's a ctl-caev_t */
		*this = ctl_wheap_pop(c->q).c;
		res.msg = this;
		res.msz = sizeof(ctl_caev_t);
		yield(res);
	}
	return 0;
}

declcoru(co_appl_wrr, {
		bool absp;
		signed int prec;
	}, {
		const struct echs_fund_s *rdr;
		const struct echs_fund_s *adj;
	});

static const void*
defcoru(co_appl_wrr, ia, arg)
{
/* no yield */
	const bool absp = ia->absp;
	const signed int prec = ia->prec;

	if (!absp) {
		while (arg != NULL) {
			_Decimal32 prc = arg->rdr->f[0U];

			pr_ei(arg->adj->t);

			if (UNLIKELY(prec)) {
				/* come up with a new raw value */
				int tgtx = quantexpd32(prc) + prec;
				prc = scalbnd32(1.df, tgtx);
			}
			fputc('\t', stdout);
			pr_d32(quantized32(arg->adj->f[0U], prc));

			for (size_t i = 1U; i < arg->adj->nf; i++) {
				/* print the rest without prec scaling */
				fputc('\t', stdout);
				pr_d32(arg->adj->f[i]);
			}
			fputc('\n', stdout);
			arg = yield(NULL);
		}
	} else /*if (absp)*/ {
		const _Decimal32 scal = mkscal(prec);

		/* absolute precision mode */
		while (arg != NULL) {
			pr_ei(arg->adj->t);

			fputc('\t', stdout);
			pr_d32(quantized32(arg->adj->f[0U], scal));

			for (size_t i = 1U; i < arg->adj->nf; i++) {
				/* print the rest without prec scaling */
				fputc('\t', stdout);
				pr_d32(arg->adj->f[i]);
			}
			fputc('\n', stdout);
			arg = yield(NULL);
		}
	}
	return 0;
}

declcoru(co_appl_adj, {
		bool totret;
	}, {
		const struct echs_fund_s *rdr;
		union {
			const ctl_caev_t *c;
			float f;
		} adj_param;
	});

static const struct echs_fund_s*
defcoru(co_appl_adj, ia, arg)
{
	const bool totret = ia->totret;
	/* we'll yield a echs_fund_s */
	struct echs_fund_s res;
	size_t nf;

	if (UNLIKELY(arg == NULL)) {
		return 0;
	} else if ((nf = res.nf = arg->rdr->nf) == 0U || nf > 3U) {
		return 0;
	}

	if (!totret) {
		switch (nf) {
		case 1U:
			do {
				const ctl_caev_t *c = arg->adj_param.c;
				ctl_price_t adj;

				assert(c != NULL);
				adj = ctl_caev_act_mktprc(*c, arg->rdr->f[0U]);
				res.t = arg->rdr->t;
				res.f[0U] = adj;
			} while ((arg = yield(res)) != NULL);
			break;
		case 2U:
		case 3U:
			do {
				const ctl_caev_t *c = arg->adj_param.c;
				ctl_fund_t fnd;
				ctl_fund_t adj;

				assert(c != NULL);

				fnd.mktprc = arg->rdr->f[0U];
				fnd.nomval = arg->rdr->f[nf - 2U];
				fnd.outsec = arg->rdr->f[nf - 1U];

				adj = ctl_caev_act(*c, fnd);
				res.t = arg->rdr->t;
				res.f[nf - 2U] = adj.nomval;
				res.f[nf - 1U] = adj.outsec;
				res.f[0U] = adj.mktprc;
			} while ((arg = yield(res)) != NULL);
			break;
		}

	} else /* if (totret)*/ {
		switch (nf) {
		case 1U:
			do {
				const float fctr = arg->adj_param.f;

				res.t = arg->rdr->t;
				res.f[0U] = (float)arg->rdr->f[0U] * fctr;
			} while ((arg = yield(res)) != NULL);
			break;
		case 2U:
		case 3U:
			do {
				const float fctr = arg->adj_param.f;

				res.t = arg->rdr->t;
				res.f[nf - 2U] =
					(float)arg->rdr->f[nf - 2U] / fctr;
				res.f[nf - 1U] =
					(float)arg->rdr->f[nf - 1U] / fctr;
				res.f[0U] = (float)arg->rdr->f[0U] * fctr;
			} while ((arg = yield(res)) != NULL);
			break;
		}
	}

	return 0;
}

static struct echs_fund_s
massage_adj(const struct echs_fund_s *msg)
{
/* massage a message from the adj coru into something more useful */
	return *msg;
}


/* public api, might go to libcattle one day */
static int
ctl_read_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
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
	rdr = make_coru(co_appl_rdr, f);

	for (const struct rdr_res_s *ln; (ln = next(rdr));) {
		/* try to read the whole shebang */
		ctl_caev_t c = ctl_caev_rdr(ln->t, ln->ln);

		/* insert to heap */
		ctl_wheap_add_deferred(ctx->q, ln->t, (colour_t){c});
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(ctx->q);
	free_coru(rdr);
	fclose(f);
	fini_coru();
	return 0;
}

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
	rdr = make_coru(co_appl_rdr, f);

	for (const struct rdr_res_s *ln; (ln = next(rdr));) {
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

static int
ctl_appl_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices */
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	ctl_caev_t sum;
	FILE *f;

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = false);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	if (!ctx->fwd) {
		sum = ctl_caev_sum(ctx->q);
		if (ctx->rev) {
			sum = ctl_caev_inv(sum);
		}
	} else {
		sum = ctl_zero_caev();
	}

	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
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
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r,
						.adj_param.c = &sum));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				pack_args(co_appl_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));
	}
	/* drain the adjuster, then the writer */
	(void)next(adj);
	(void)next(wrr);

	free_coru(rdr);
	free_coru(pop);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	fclose(f);
	return 0;
}

static int
ctl_fadj_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return forward adjustment */
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	float prod;
	int res = 0;
	FILE *f;

	assert(ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f);
	pop = make_coru(co_appl_pop, ctx->q);
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	/* initialise product */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop)) {
			ctl_caev_t caev;
			float fctr;
			float aadj;

			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			caev = *(const ctl_caev_t*)ev->msg;
			aadj = (float)caev.mktprc.a;

			if (UNLIKELY(ctx->rev)) {
				/* last price needs adaption */
				last *= prod;
			}
			fctr = 1.f + aadj / last;
			fctr *= ratio_to_float(caev.mktprc.r);
			if (!ctx->rev) {
				prod /= fctr;
			} else {
				prod *= fctr;
			}
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			/* save last value */
			last = (float)r.f[0U];

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r, .adj_param.f = prod));
				a = massage_adj(tmp);
			}
			/* off to the writer */
			next_with(
				wrr,
				pack_args(co_appl_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));
	}
	/* unload the writer and adjuster */
	(void)next(adj);
	(void)next(wrr);

out:
	/* finished, yay */
	free_coru(pop);
	free_coru(rdr);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	fclose(f);
	return res;
}

static int
ctl_badj_caev_file(struct ctl_ctx_s ctx[static 1U], const char *fn)
{
/* wants a const char *fn, the time series
 * format in there is first column is a date, the rest is prices
 * this is the total return forward adjustment */
	struct fa_s {
		echs_instant_t t;
		float fctr;
		float aadj;
		float last;
	};
	coru_t rdr;
	coru_t pop;
	coru_t adj;
	coru_t wrr;
	struct fa_s *fa = NULL;
	size_t nfa = 0U;
	float prod;
	int res = 0;
	FILE *f;

	assert(!ctx->fwd);

	if (fn == NULL || fn[0U] == '-' && fn[1U] == '\0') {
		f = stdin;
	} else if (UNLIKELY((f = fopen(fn, "r")) == NULL)) {
		return -1;
	}

	init_coru();
	rdr = make_coru(co_appl_rdr, f, .loop = 1U);
	pop = make_coru(co_appl_pop, ctx->q);

	/* initialise another wheap and another prod */
	prod = 1.f;

	float last = NAN;
	const struct pop_res_s *ev;
	const struct rdr_res_s *ln;
	for (ln = next(rdr), ev = next(pop); ln != NULL;) {
		/* skip over events from the past,
		 * i.e. from before the time of the first market price */
		for (; LIKELY(ev != NULL) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, ev->t));
		     ev = next(pop));

		do {
			/* no need to use the strtod32() reader */
			last = strtof(ln->ln, NULL);
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(ev == NULL || echs_instant_lt_p(ln->t, ev->t)));

		/* sum up caevs in between price lines */
		for (;
		     LIKELY(ev != NULL) &&
			     (UNLIKELY(ln == NULL) ||
			      UNLIKELY(!echs_instant_lt_p(ln->t, ev->t)));
		     ev = next(pop)) {
			ctl_caev_t caev;
			struct fa_s cell;

			caev = *(const ctl_caev_t*)ev->msg;
			cell.last = last;
			cell.aadj = (float)caev.mktprc.a;
			cell.t = ev->t;

			/* represent everything as factor */
			if (LIKELY(!ctx->rev)) {
				/* all's good */
				cell.fctr = 1.f + cell.aadj / cell.last;
			} else {
				/* we do the fix up later ...
				 * there's nothing else we need */
				cell.fctr = 1.f;
			}
			cell.fctr *= ratio_to_float(caev.mktprc.r);
			prod *= cell.fctr;

			/* push to fa array */
			if ((nfa % 64U) == 0U) {
				fa = realloc(fa, (nfa + 64) * sizeof(*fa));
			}
			fa[nfa++] = cell;
		}
	}

	/* end of first pass */
	free_coru(pop);

	if (UNLIKELY(ctx->rev)) {
		/* massage the factors,
		 * we know the last factor is correct, we don't know
		 * about the N-1 factors before that though
		 * so we reinstantiate the raw price P[N-1] using the
		 * last factor and improve the (N-1)-th factor
		 * and so on */
		for (size_t i = nfa - 1U; i < nfa; i--) {
			float p = 1.f;

			/* determine sub-prod */
			for (size_t j = i + 1U; j < nfa; j++) {
				p *= fa[j].fctr;
			}
			/* unadjust i-th last price */
			fa[i].last *= p;
			/* improve i-th factor now, store inverse,
			 * use identity 1 + d/x = (x + d) / x */
			fa[i].fctr = (fa[i].last - fa[i].aadj) / fa[i].last /
				fa[i].fctr;
		}

		/* (re)calc prod */
		prod = 1.f;
		for (size_t i = 0; i < nfa; i++) {
			prod *= fa[i].fctr;
		}
	}

	/* last pass, we reuse the RDR coru as it's in loop mode */
	adj = make_coru(co_appl_adj, .totret = true);
	wrr = make_coru(co_appl_wrr, .absp = ctx->abs_prec, .prec = ctx->prec);

	last = NAN;
	size_t i;
	for (ln = next(rdr), i = 0U; ln != NULL;) {
		/* mul up factors in between price lines */
		for (;
		     LIKELY(i < nfa) &&
			     UNLIKELY(!echs_instant_lt_p(ln->t, fa[i].t));
		     i++) {
			if (UNLIKELY(isnan(last))) {
				res = -1;
				goto out;
			}

			/* compute the new adjustment factor */
			prod /= fa[i].fctr;
		}

		/* apply caev sum to price lines */
		do {
			struct echs_fund_s r = massage_rdr(ln);
			struct echs_fund_s a;

			/* save last value */
			last = (float)r.f[0U];

			with (const struct echs_fund_s *tmp) {
				tmp = next_with(
					adj,
					pack_args(
						co_appl_adj,
						.rdr = &r,
						.adj_param.f = prod));
				a = massage_adj(tmp);
			}

			/* off to the writer */
			next_with(
				wrr,
				pack_args(co_appl_wrr, .rdr = &r, .adj = &a));
		} while (LIKELY((ln = next(rdr)) != NULL) &&
			 LIKELY(i >= nfa || echs_instant_lt_p(ln->t, fa[i].t)));
	}
	/* unload the writer and adjuster */
	(void)next(adj);
	(void)next(wrr);

out:
	/* finished, yay */
	free_coru(rdr);
	free_coru(adj);
	free_coru(wrr);
	fini_coru();

	if (nfa > 0U) {
		assert(fa != NULL);
		free(fa);
	}

	fclose(f);
	return res;
}

/* printer commands */
static int
ctl_print_raw(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t prev = ctl_zero_caev();
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q)); prev_t = t) {
		ctl_caev_t this = ctl_wheap_pop(ctx->q).c;
		char buf[256U];
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		if (uniqp) {
			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_eq_p(this, prev)) {
				/* completely identical */
				continue;
			}
		}

		bp += dt_strf(bp, ep - bp, t);
		*bp++ = '\t';
		bp += ctl_caev_wr(bp, ep - bp, this);
		prev = this;
		*bp++ = '\n';
		*bp = '\0';
		fputs(buf, stdout);
	}
	return res;
}

static int
ctl_print_sum(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t sum = ctl_zero_caev();
	ctl_caev_t prev = sum;
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q)); prev_t = t) {
		ctl_caev_t this = ctl_wheap_pop(ctx->q).c;

		if (uniqp) {
			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_eq_p(this, prev)) {
				/* completely identical */
				continue;
			}
		}
		/* just sum them up here */
		sum = ctl_caev_add(sum, this);
		prev = this;
	}
	/* and now print */
	{
		char buf[256U];
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		bp += ctl_caev_wr(bp, ep - bp, sum);
		*bp++ = '\n';
		*bp = '\0';
		fputs(buf, stdout);
	}
	return res;
}

static int
ctl_print_kv(struct ctl_ctx_s ctx[static 1U], bool uniqp)
{
	ctl_caev_t prev_c = ctl_zero_caev();
	echs_instant_t prev_t = {.u = 0U};
	int res = 0;

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(ctx->q)); prev_t = t) {
		ctl_kvv_t this = ctl_wheap_pop(ctx->q).flds;
		char buf[256U];
		char *bp = buf;
		const char *const ep = buf + sizeof(buf);

		if (uniqp) {
			ctl_caev_t this_c = ctl_kvv_get_caev(this);

			if (echs_instant_eq_p(prev_t, t) &&
			    ctl_caev_eq_p(this_c, prev_c)) {
				continue;
			}
			/* store actor representation for the next round */
			prev_c = this_c;
		}

		bp += dt_strf(bp, ep - bp, t);
		*bp++ = '\t';
		bp += ctl_kvv_wr(bp, ep - bp, this);
		free_kvv(this);
		*bp++ = '\n';
		*bp = '\0';
		fputs(buf, stdout);
	}
	return res;
}


#if defined STANDALONE
#include "cattle.yucc"

static int
cmd_print(const struct yuck_cmd_print_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	bool rawp = argi->raw_flag;
	bool uniqp = argi->unique_flag;
	int rc = 1;

	if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		goto out;
	} else if (argi->summary_flag) {
		/* --summary implies --raw */
		rawp = true;
	}

	if (argi->nargs == 0U && !rawp) {
		if (UNLIKELY(ctl_read_kv_file(ctx, NULL) < 0)) {
			error("cannot read from stdin");
			goto out;
		}
	} else if (argi->nargs == 0U) {
		if (UNLIKELY(ctl_read_caev_file(ctx, NULL) < 0)) {
			error("cannot read from stdin");
			goto out;
		}
	}
	for (size_t i = 0U; i < argi->nargs && !rawp; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_kv_file(ctx, fn) < 0)) {
			error("cannot open file `%s'", fn);
			goto out;
		}
	}
	for (size_t i = 0U; i < argi->nargs && rawp; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caev_file(ctx, fn) < 0)) {
			error("cannot open file `%s'", fn);
			goto out;
		}
	}

	if (!rawp && ctl_print_kv(ctx, uniqp) >= 0) {
		rc = 0;
	} else if (argi->summary_flag && ctl_print_sum(ctx, uniqp) >= 0) {
		rc = 0;
	} else if (rawp && ctl_print_raw(ctx, uniqp) >= 0) {
		rc = 0;
	}

out:
	if (LIKELY(ctx->q != NULL)) {
		free_ctl_wheap(ctx->q);
	}
	return rc;
}

static int
cmd_apply(const struct yuck_cmd_apply_s argi[static 1U])
{
	static struct ctl_ctx_s ctx[1];
	int res = 0;

	if (argi->nargs < 1U) {
		yuck_auto_help((const void*)argi);
		res = 1;
		goto out;
	} else if (UNLIKELY((ctx->q = make_ctl_wheap()) == NULL)) {
		res = 1;
		goto out;
	}

	if (argi->reverse_flag) {
		ctx->rev = 1U;
	}
	if (argi->forward_flag) {
		ctx->fwd = 1U;
	}
	if (argi->precision_arg) {
		const char *p = argi->precision_arg;
		char *on;

		if (*p == '+' || *p == '-') {
			;
		} else {
			ctx->abs_prec = 1U;
		}
		if ((ctx->prec = -strtol(p, &on, 10), *on)) {
			error("invalid precision `%s'", argi->precision_arg);
			res = 1;
			goto out;
		}
	}

	/* open caev files and read */
	if (argi->nargs <= 1U) {
		if (UNLIKELY(ctl_read_caev_file(ctx, NULL) < 0)) {
			error("cannot read from stdin");
			res = 1;
			goto out;
		}
	}
	for (size_t i = 1U; i < argi->nargs; i++) {
		const char *fn = argi->args[i];

		if (UNLIKELY(ctl_read_caev_file(ctx, fn) < 0)) {
			error("cannot open caev file `%s'", fn);
			res = 1;
			goto out;
		}
	}

	/* open time series file */
	with (const char *tser_fn = argi->args[0U]) {
		if (argi->total_return_flag && !ctx->fwd) {
			/* total return back adjustment needs 2 scans */
			if (UNLIKELY(ctl_badj_caev_file(ctx, tser_fn) < 0)) {
				error("\
cannot deduce factors for total return adjustment from `%s'", tser_fn);
				res = 1;
				goto out;
			}
		} else if (argi->total_return_flag/* && ctx->fwd */) {
			if (UNLIKELY(ctl_fadj_caev_file(ctx, tser_fn) < 0)) {
				error("\
cannot deduce factors for total return adjustment from `%s'", tser_fn);
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
	return res;
}

int
main(int argc, char *argv[])
{
	yuck_t argi[1U];
	int res = 0;

	if (yuck_parse(argi, argc, argv)) {
		res = 99;
		goto out;
	}

	/* get the coroutines going */
	init_coru_core();

	/* check the command */
	switch (argi->cmd) {
	default:
	case CATTLE_CMD_NONE:
		error("No valid command specified.\n\
See --help to obtain a list of available commands.");
		res = 1;
		break;

	case CATTLE_CMD_APPLY:
		res = cmd_apply((const void*)argi);
		break;

	case CATTLE_CMD_PRINT:
		res = cmd_print((const void*)argi);
		break;
	}

out:
	/* just to make sure */
	fflush(stdout);
	yuck_free(argi);
	return res;
}
#endif	/* STANDALONE */

/* cattle.c ends here */
