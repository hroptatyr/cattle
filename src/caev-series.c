/*** caev-series.c -- time series of caevs
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
 **/
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/mman.h>
#include "caev.h"
#include "caev-series.h"
#include "caev-rdr.h"
#include "coru.h"
#include "dt-strpf.h"
#include "nifty.h"

/* lightweight wheap decl */
struct ctl_wheap_s {
	/** number of cells on the heap */
	size_t n;
	/** the cells themselves, with < defined by __inst_lt_p() */
	echs_instant_t *cells;
	colour_t *colours;
};


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


const struct ctl_co_rdr_res_s*
defcoru(ctl_co_rdr, c, UNUSED(arg))
{
/* coroutine for the reader of the tseries */
	char *line = NULL;
	size_t llen = 0UL;
	/* we'll yield a rdr_res */
	struct ctl_co_rdr_res_s res;
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


/* public api */
ctl_caev_t
ctl_caev_sum(ctl_caevs_t cs)
{
	ctl_caev_t sum = ctl_zero_caev();

	ctl_wheap_sort(cs);
	for (size_t i = 0; i < cs->n; i++) {
		sum = ctl_caev_add(sum, cs->colours[i].c);
	}
	return sum;
}

int
ctl_read_caevs(ctl_caevs_t q, const char *fn)
{
/* wants a const char *fn */
	coru_t rdr;
	FILE *f;

	/* initialise coru core singleton */
	init_coru_core();

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
		ctl_wheap_add_deferred(q, ln->t, (colour_t){.flds = v});
	}
	/* now sort the guy */
	ctl_wheap_fix_deferred(q);
	free_coru(rdr);
	fclose(f);
	fini_coru();
	return 0;
}

/* caev-series.c ends here */
