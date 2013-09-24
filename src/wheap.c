/*** wheap.c -- weak heaps, as stolen from sxemacs
 *
 * Copyright (C) 2007-2013 Sebastian Freundt
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
/**
 * Weak heaps according to:
 * The Weak-Heap Data Structure, Variants and Applications
 * Stefan Edelkamp, Amr Elmasry, Jyrki Katajainen
 * Jan 2013, Elsevier */
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "instant.h"
#include "wheap.h"
#include "nifty.h"

typedef uint_fast32_t rbitset_t;

struct ctl_wheap_s {
	/** number of cells on the heap */
	size_t n;
	/** the cells themselves, with < defined by __inst_lt_p() */
	echs_instant_t *cells;
	uintptr_t *colours;
	rbitset_t *rbits;
#define RBITS_WIDTH	(sizeof(rbitset_t) * 8U)

	/** number of recent bulk inserts (deferred adds)
	 * also used as an indicator for the heap property, 0 means yes. */
	size_t ndfr;
	/** allocated size */
	size_t z;
};


#define recalloc(p, ol, nu)						\
	({								\
		(p) = realloc((p), (nu) * sizeof(*(p)));		\
		if ((nu) > (ol)) {					\
			memset((p) + (ol), 0, ((nu) - (ol)) * sizeof(*(p))); \
		}							\
		p;							\
	})


static void
__wheap_resz(ctl_wheap_t h, size_t nu_z)
{
	/* round nu_z to multiple of wid */
	nu_z = ((nu_z - 1U) / RBITS_WIDTH + 1U) * RBITS_WIDTH;
	h->cells = recalloc(h->cells, h->z, nu_z);
	h->colours = recalloc(h->colours, h->z, nu_z);
	h->rbits = recalloc(h->rbits, h->z / RBITS_WIDTH, nu_z / RBITS_WIDTH);
	h->z = nu_z;
	return;
}

/* weak heap navigation */
static inline unsigned int
__wheap_cell_rbit(ctl_wheap_t h, size_t i)
{
	size_t cidx = i / RBITS_WIDTH, bidx = i % RBITS_WIDTH;

	return (h->rbits[cidx] >> bidx) & 1U;
}

static inline unsigned int
__wheap_cell_rneg(ctl_wheap_t h, size_t i)
{
	size_t cidx = i / RBITS_WIDTH, bidx = i % RBITS_WIDTH;

	return h->rbits[cidx] ^= (1ULL << bidx);
}

static inline void
__wheap_void_rbit(ctl_wheap_t h, size_t i)
{
	size_t cidx = i / RBITS_WIDTH, bidx = i % RBITS_WIDTH;

	h->rbits[cidx] &= ~(1ULL << bidx);
	return;
}

static inline __attribute__((pure, const)) size_t
__wheap_cell_dad(ctl_wheap_t UNUSED(h), size_t i)
{
	return i >> 1U;
}

static inline void
__wheap_void_dad(ctl_wheap_t h, size_t i)
{
	__wheap_void_rbit(h, __wheap_cell_dad(h, i));
	return;
}

static inline size_t
__wheap_cell_pop(ctl_wheap_t h, size_t i)
{
/* called d-ancestor in Edelkamp's improved paper */
	while ((i & 1U) == __wheap_cell_rbit(h, __wheap_cell_dad(h, i))) {
		i = __wheap_cell_dad(h, i);
	}
	return __wheap_cell_dad(h, i);
}

static inline size_t
__wheap_cell_bro(ctl_wheap_t h, size_t i)
{
/* brothers are to the left */
	return 2U * i + __wheap_cell_rbit(h, i);
}

static inline size_t
__wheap_cell_sis(ctl_wheap_t h, size_t i)
{
/* sisters are to the right */
	return 2U * i + 1U - __wheap_cell_rbit(h, i);
}

/* swapping */
#define array_swap(arr, i, j)			\
	do {					\
		typeof(*arr) tmp = (arr)[i];	\
		(arr)[i] = (arr)[j];		\
		(arr)[j] = tmp;			\
	} while (0)

static inline void
__wheap_swap(ctl_wheap_t h, size_t i, size_t j)
{
	/* swap priority data */
	array_swap(h->cells, i, j);
	/* swap colours too */
	array_swap(h->colours, i, j);
	return;
}

/* merging */
static bool
__wheapify_mrg(ctl_wheap_t h, size_t i, size_t j)
{
/* aka Merge(i, j) in Edelkamp/Wegener's paper, or join in the improved one */
	bool res;

	if ((res = __inst_lt_p(h->cells[j], h->cells[i]))) {
		/* swap(pop(idx), idx) */
		__wheap_swap(h, i, j);
		/* update bit field */
		__wheap_cell_rneg(h, j);
	}
	return !res;
}

static void
__wheapify_mrgfor(ctl_wheap_t h, size_t m)
{
/* aka MergeForest(m) in Edelkamp/Wegener's paper */
	size_t x = 1U;

	if (UNLIKELY(m <= 1U)) {
		return;
	}

	for (size_t l; (l = __wheap_cell_bro(h, x)) < m; x = l);
	for (; x > 0; x = __wheap_cell_dad(h, x)) {
		/* merge(m, x), then move on to parent */
		__wheapify_mrg(h, m, x);
	}
	return;
}

static void
__wheapify_sift_up(ctl_wheap_t h, size_t j)
{
/* aka sift-up(j) in Edelkamp's improved paper */
	for (size_t i;
	     j > 0U && (
		     i = __wheap_cell_pop(h, j),
		     !__wheapify_mrg(h, i, j)
		     ); j = i);
	return;
}

static void
__wheapify_sift_down(ctl_wheap_t h, size_t j)
{
/* aka sift-down(j) in Edelkamp's improved paper */
	size_t k;

	if ((k = __wheap_cell_sis(h, j)) >= h->n) {
		return;
	}
	for (size_t nu; (nu = __wheap_cell_bro(h, k)) < h->n; k = nu);
	for (; k != j; k = __wheap_cell_dad(h, k)) {
		__wheapify_mrg(h, j, k);
	}
	return;
}

static void
__wheapify_dfr(ctl_wheap_t h)
{
/* assume the last K elements of H have been ins'd by _add_dfr() */
	size_t n;
	size_t r;
	size_t l;

	if (UNLIKELY(h->ndfr == 0U)) {
		/* say what? */
		return;
	} else if (UNLIKELY((n = h->n - h->ndfr) > h->n)) {
		/* not sure what happened */
		h->ndfr = 0U;
		return;
	}

	/* left/right ass'ment */
	r = n + h->ndfr - 1U;
	if ((l = __wheap_cell_dad(h, r)) < n) {
		/* l = max{dad(r), n} */
		l = n;
	}
	while (r > l + 1U) {
		l = __wheap_cell_dad(h, l);
		r = __wheap_cell_dad(h, r);
		for (size_t j = l; j <= r; j++) {
			__wheapify_sift_down(h, j);
		}
	}
	if (r > 0U) {
		size_t pop = __wheap_cell_pop(h, r);
		__wheapify_sift_down(h, pop);
	}
	if (l > 0U) {
		size_t pop = __wheap_cell_pop(h, l);
		__wheapify_sift_down(h, pop);
		__wheapify_sift_up(h, pop);
	}
	__wheapify_sift_up(h, 0U);
	return;
}

static size_t
wheap_add_dfr(ctl_wheap_t h, echs_instant_t inst, uintptr_t msg)
{
	size_t idx;

	/* check for resize */
	if (UNLIKELY((idx = h->n) + 1U >= h->z)) {
		__wheap_resz(h, h->z * 2U);
	}

	h->cells[idx] = inst;
	h->colours[idx] = msg;

	/* we now violate the heap property */
	h->ndfr++;
	h->n++;
	return idx;
}

static size_t
wheap_add(ctl_wheap_t h, echs_instant_t inst, uintptr_t msg)
{
	size_t idx;

#if defined AUTO_FIXUP_BULK_OPS
	/* check for unfixed bulks */
	if (UNLIKELY(h->ndfr > 0U)) {
		__wheapify_dfr(h);
	}
#endif	/* AUTO_FIXUP_BULK_OPS */
	/* check for resize */
	if (UNLIKELY((idx = h->n) + 1U >= h->z)) {
		__wheap_resz(h, h->z * 2U);
	}

	h->cells[idx] = inst;
	h->colours[idx] = msg;
	__wheap_void_rbit(h, idx);

	if (!(idx & 0x1U)) {
		__wheap_void_dad(h, idx);
	}

	__wheapify_sift_up(h, idx);
	h->n++;
	return idx;
}

static uintptr_t
wheap_pop(ctl_wheap_t h)
{
	uintptr_t res;
	size_t end_idx;

	if (UNLIKELY(h->n == 0U)) {
		return 0U;
#if defined AUTO_FIXUP_BULK_OPS
	} else if (UNLIKELY(h->ndfr > 0U)) {
		/* fix up bulk inserts? */
		__wheapify_dfr(h);
#endif	/* AUTO_FIXUP_BULK_OPS */
	}

	res = h->colours[0U];

	end_idx = --h->n;

	h->cells[0U] = h->cells[end_idx];
	h->cells[end_idx] = (echs_instant_t){};

	h->colours[0U] = h->colours[end_idx];
	h->colours[end_idx] = (uintptr_t)NULL;

	if (end_idx > 1) {
		__wheapify_sift_down(h, 0U);
	}
	return res;
}

static uintptr_t
wheap_top(ctl_wheap_t h)
{
	if (UNLIKELY(h->n == 0U)) {
		return 0U;
#if defined AUTO_FIXUP_BULK_OPS
	} else if (UNLIKELY(h->ndfr > 0U)) {
		/* fix up bulk inserts? */
		__wheapify_dfr(h);
#endif	/* AUTO_FIXUP_BULK_OPS */
	}

	return h->colours[0U];
}

static echs_instant_t
wheap_top_rank(ctl_wheap_t h)
{
	if (UNLIKELY(h->n == 0U)) {
		return (echs_instant_t){};
#if defined AUTO_FIXUP_BULK_OPS
	} else if (UNLIKELY(h->ndfr > 0U)) {
		/* fix up bulk inserts? */
		__wheapify_dfr(h);
#endif	/* AUTO_FIXUP_BULK_OPS */
	}

	return h->cells[0U];
}

static void
wheap_sort(ctl_wheap_t h)
{
	if (UNLIKELY(h->n == 0U)) {
		return;
	}

	/* normally WeakHeapify is called first
	 * howbeit, our wheaps always suffice the weak property */
	for (size_t s = h->n - 1U; s > 1U; s--) {
		__wheapify_mrgfor(h, s);
	}
	/* now the i-th most extreme value is at index h->n - i */
	with (const size_t lim = __wheap_cell_dad(h, h->n + 1U)) {
		for (size_t i = 1, j = h->n - 1U; i < lim; i++, j--) {
			__wheap_swap(h, i, j);
		}
	}
	h->ndfr = 0U;
	return;
}


ctl_wheap_t
make_ctl_wheap(void)
{
	ctl_wheap_t h;

	if (UNLIKELY((h = malloc(sizeof(struct ctl_wheap_s))) == NULL)) {
		return NULL;
	}

	/* status so far */
	h->n = 0U;
	h->ndfr = 0U;
	/* minimum size, say, 64 innit? */
	h->z = 64U;

	h->cells = calloc(h->z, sizeof(*h->cells));
	h->colours = calloc(h->z, sizeof(*h->colours));
	h->rbits = calloc(h->z / 8U, sizeof(*h->rbits));
	return h;
}

void
free_ctl_wheap(ctl_wheap_t h)
{
	free(h->cells);
	free(h->rbits);
	free(h->colours);
	free(h);
	return;
}

echs_instant_t
ctl_wheap_top_rank(ctl_wheap_t h)
{
	return wheap_top_rank(h);
}

uintptr_t
ctl_wheap_top(ctl_wheap_t h)
{
	return wheap_top(h);
}

uintptr_t
ctl_wheap_pop(ctl_wheap_t h)
{
	return wheap_pop(h);
}

void
ctl_wheap_add(ctl_wheap_t h, echs_instant_t inst, uintptr_t msg)
{
	wheap_add(h, inst, msg);
	return;
}

void
ctl_wheap_add_deferred(ctl_wheap_t h, echs_instant_t inst, uintptr_t msg)
{
	wheap_add_dfr(h, inst, msg);
	return;
}

void
ctl_wheap_fix_deferred(ctl_wheap_t h)
{
	__wheapify_dfr(h);
	return;
}

void
ctl_wheap_sort(ctl_wheap_t h)
{
	wheap_sort(h);
	return;
}

/* wheap.c ends here */
