/*** wheap.h -- weak heaps, as stolen from sxemacs
 *
 * Copyright (C) 2007-2015 Sebastian Freundt
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
#if !defined INCLUDED_wheap_h_
#define INCLUDED_wheap_h_

#include <stdlib.h>
#include <stdint.h>
#include "instant.h"

typedef struct ctl_wheap_s *ctl_wheap_t;

#if !defined WHEAP_COLOUR_H && !defined WHEAP_COLOUR_T
typedef uintptr_t colour_t;
#endif	/* !WHEAP_COLOUR_H && !WHEAP_COLOUR_T */


extern ctl_wheap_t make_ctl_wheap(void);
extern void free_ctl_wheap(ctl_wheap_t);

/* include details about the colour of the wheap
 * this is to get the memory layout right */
#if defined WHEAP_COLOUR_H
# include WHEAP_COLOUR_H
#endif	/* WHEAP_COLOUR_H */

extern echs_instant_t ctl_wheap_top_rank(ctl_wheap_t);
extern colour_t ctl_wheap_top(ctl_wheap_t);
extern colour_t ctl_wheap_pop(ctl_wheap_t);

extern void ctl_wheap_add(ctl_wheap_t, echs_instant_t, colour_t);

/**
 * Bulk inserts. */
extern void ctl_wheap_add_deferred(ctl_wheap_t, echs_instant_t, colour_t);
/**
 * Recreate the heap property after deferred inserts. */
extern void ctl_wheap_fix_deferred(ctl_wheap_t);

/**
 * Sort the entire heap. */
extern void ctl_wheap_sort(ctl_wheap_t);

#endif	/* INCLUDED_wheap_h_ */
