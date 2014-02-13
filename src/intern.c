/*** intern.c -- interning system
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
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"
#include "intern.h"

typedef struct node_s *node_t;

struct trie_s {
	node_t root;
};

static struct trie_s intt[1U];
static char *restrict ints;
static size_t intp;
static size_t intz;


obint_t
intern(const char *str)
{
	obint_t res;
	size_t ztr;

	if ((res = (uintptr_t)trie_get(intt, str))) {
		return res;
	}
	/* otherwise intern the string */
	ztr = strlen(str);

	/* resize? */
	if (intp + ztr >= intz) {
		/* time to resize again */
		intz = (intz * 2U) ?: 256U;
		ints = realloc(ints, intz);
	}

	/* make the unique copy of STR */
	memcpy(ints + (res = intp), str, ztr);
	/* up our admin pointers */
	intp += ztr;
	/* finalise string */
	ints[intp++] = '\0';

	/* and finally leave a not in the trie */
	trie_put(intt, str, (void*)res);
	return res;
}

const char*
obint_name(obint_t ob)
{
	return ints + (ptrdiff_t)ob;
}

void
clear_interns(void)
{
	deinit_trie(intt);
	free(ints);
	ints = NULL;
	intz = 0U;
	return;
}

/* intern.c ends here */
