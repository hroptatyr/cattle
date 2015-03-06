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
