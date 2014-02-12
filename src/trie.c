/*** trie.c -- string lookups
 *
 * Copyright (C) 2013-2014 Sebastian Freundt
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
#include "trie.h"

#if !defined countof
# define countof(x)	(sizeof(x) / sizeof(*x))
#endif	/* countof */

typedef struct node_s *node_t;

struct node_s {
	void *cell;
	size_t nref;
	node_t next[128U - ' '];
};

struct trie_s {
	node_t root;
};


static inline unsigned int
slot(const char p)
{
	unsigned int res = (unsigned char)p - ' ';
	return res & 0x7fU;
}

static void
node_list_push(node_t *list, node_t nd)
{
	nd->cell = *list;
	*list = nd;
	return;
}

static node_t
node_list_pop(node_t *list)
{
	node_t res = *list;

	*list = res->cell;
	return res;
}

/* idea copied from: http://c-algorithms.sourceforge.net/ */
static void
__rollback(trie_t t, const char *key)
{
	/* Follow the chain along.  We know that we will never reach the 
	 * end of the string because trie_insert never got that far.  As a
	 * result, it is not necessary to check for the end of string
	 * delimiter (NUL) */
	for (node_t n = t->root, next, *prev = &t->root, *next_prev;
	     n != NULL; n = next, prev = next_prev) {
		/* Find the next node now. We might free this node. */
		next_prev = &n->next[slot(*key++)];
		next = *next_prev;

		/* Decrease the use count and free the node if it 
		 * reaches zero. */
		if (--n->nref == 0U) {
			/* get rid of him */
			free(n);

			if (prev != NULL) {
				*prev = NULL;
			}
			next_prev = NULL;
		}
	}
	return;
}

static node_t
__find(trie_t t, const char *key)
{
	node_t n = t->root;

	for (const char *kp = key; *kp; kp++) {
		if (n == NULL) {
			/* not in my trie it isn't */
			return NULL;
		}
		/* just branch down to the next node */
		n = n->next[slot(*kp)];
	}
	/* found him */
	return n;
}


void
init_trie(struct trie_s *restrict t)
{
	t->root = NULL;
	return;
}

void
deinit_trie(struct trie_s *restrict t)
{
	node_t free_nodes = NULL;

	if (t->root == NULL) {
		return;
	}

	if (t->root != NULL) {
		node_list_push(&free_nodes, t->root);
	}


	/* traverse free list indefinitely, add nodes as we pass them */
	while (free_nodes != NULL) {
		node_t n = node_list_pop(&free_nodes);

		/* append all children */
		for (size_t i = 0U; i < countof(n->next); i++) {
			if (n->next[i] != NULL) {
				node_list_push(&free_nodes, n);
			}
		}
		/* finally free the node */
		free(n);
	}
	return;
}

trie_t
make_trie(void)
{
	trie_t res;

	if ((res = calloc(1U, sizeof(*res))) == NULL) {
		return NULL;
	}
	return res;
}

void
free_trie(trie_t t)
{
	deinit_trie(t);
	free(t);
	return;
}

int
trie_put(trie_t t, const char *key, void *val)
{
	node_t *np;
	node_t n;

	if (val == NULL) {
		/* bugger off, user is trying to trick us, aren't they? */
		return -1;
	}
		
	/* have we got KEY already? */
	if ((n = __find(t, key)) != NULL && n->cell != NULL) {
		/* bingo, just replace the cell values */
		n->cell = val;
		return 0;
	}

	/* go down branches till eokey, pluck in nodes if need be */
	np = &t->root;
	for (const char *kp = key;;) {
		if ((n = *np) == NULL) {
			/* nope, create node */
			if ((n = calloc(1U, sizeof(*n))) == NULL) {
				/* big cluster fuck */
				__rollback(t, key);
				return -1;
			}
			/* link in to the trie */
			*np = n;
		}

		/* up ref counter */
		n->nref++;

		/* eo-string? */
		if (!*kp) {
			/* set cell slot to val and bugger off */
			n->cell = val;
			break;
		}

		/* Advance to the next node in the chain */
		np = &n->next[slot(*kp++)];
	}
	return 1;
}

void*
trie_get(trie_t t, const char *key)
{
	node_t n;

	if ((n = __find(t, key)) == NULL) {
		return NULL;
	}
	return n->cell;
}

void*
trie_del(trie_t t, const char *key)
{
	void *res;

	for (node_t n;;) {
		/* find end node and remove value */
		if ((n = __find(t, key)) == NULL || (res = n->cell) == NULL) {
			/* not found at all */
			return NULL;
		}
		/* traverse trie again, update ref counters */
		n->cell = NULL;
		break;
	}

	for (node_t n = t->root, next, *nextp = &t->root;; key++, n = next) {
		/* find next node already so if need be we can free N */
		next = n->next[slot(*key)];

		/* down the ref counter and free this node if need be */
		if (--n->nref == 0U) {
			free(n);

			/* Set "next" pointer on the previous node to NULL,
			 * to unlink the freed node from the tree.  This only
			 * needs to be done once in a remove.  After the first
			 * unlink, all further nodes are also going to be
			 * free'd. */
			if (nextp != NULL) {
				*nextp = NULL;
				nextp = NULL;
			}
		}
		if (!*key) {
			break;
		} else if (nextp != NULL) {
			nextp = &n->next[slot(*key)];
		}
	}

	/* removed successfully */
	return res;
}

/* trie.c ends here */
