/*** coru.h -- coroutines
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
#if !defined INCLUDED_coru_h_
#define INCLUDED_coru_h_

#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */

#define declcoru(name, init...)	struct name##_initargs_s init

#if !defined _paste
# define _paste(x, y)		x ## y
#endif	/* !_paste */
#if !defined paste
# define paste(x, y)		_paste(x, y)
#endif	/* !paste */
#define TMP(x)			paste(__##x##__, __LINE__)

#if defined USE_ASM_CORUS
#include "coru/cocore.h"

typedef struct cocore *coru_t;

static __thread coru_t ____caller;

#define init_coru_core(args...)	initialise_cocore()
#define init_coru()		____caller = initialise_cocore_thread()
#define fini_coru()		terminate_cocore_thread(), ____caller = NULL

#define make_coru(x, init...)						\
	({								\
		struct x##_initargs_s TMP(initargs) = {init};		\
		create_cocore(						\
			____caller, (cocore_action_t)(x),		\
			&TMP(initargs), sizeof(TMP(initargs)),		\
			____caller, 0U, false, 0);			\
	})								\

#define free_coru(x)

#define next(x)			____next(x, NULL)
#define next_with(x, val)				\
	({						\
		static typeof((val)) TMP(res);		\
							\
		TMP(res) = val;				\
		____next(x, &TMP(res));			\
	})
#define ____next(x, ptr)				\
	(check_cocore(x)				\
	 ? switch_cocore((x), ptr)			\
	 : NULL)

#define yield(yld)				\
	({					\
		coru_t TMP(tmp) = ____caller;	\
		yield_to(TMP(tmp), yld);	\
	})
#define yield_to(x, yld)				\
	({						\
		static typeof((yld)) TMP(res);		\
							\
		TMP(res) = yld;				\
		switch_cocore((x), (void*)&TMP(res));	\
	})


#else  /* !USE_ASM_CORUS */
/* my own take on things */
#include <setjmp.h>
#include <stdint.h>
#include <ucontext.h>

typedef struct {
	void *stk;
	size_t ssz;
	jmp_buf *jb;
} coru_t;

#define init_coru_core(args...)
#define init_coru()
#define fini_coru()

static __thread coru_t ____caller;
static __thread coru_t ____callee;
static intptr_t ____glob;

#if !defined _setjmp
# define _setjmp		setjmp
#endif	/* !_setjmp */
#if !defined _longjmp
# define _longjmp		longjmp
#endif	/* !_longjmp */

struct trampo_s {
	coru_t *c;
	jmp_buf *caller;
	const void*(*action)();
	void *initargs;
};

union trampint_u {
	const struct trampo_s *tr;
	int i[4U];
};

static void __attribute__((noreturn))
trampoline(int i0, int i1, int i2, int i3)
{
	union trampint_u ap = {.i[0] = i0, .i[1] = i1, .i[2] = i2, .i[3] = i3};
	const struct trampo_s *tr = ap.tr;

	/* set up jump environment and yield back to caller */
	if (!_setjmp(*tr->c->jb)) {
		_longjmp(*tr->caller, 1);
	}

	/* and do our thing here */
	tr->action((void*)____glob, tr->initargs);
	____glob = (intptr_t)NULL;
	_longjmp(*____caller.jb, 1);
	assert(0);
}

#define make_coru(x, init...)					\
	({							\
		static jmp_buf __##x##b;			\
		static jmp_buf __##x##trampo;			\
		static coru_t TMP(res) = {.jb = &__##x##b};	\
		static struct trampo_s tr[1];			\
		struct x##_initargs_s TMP(initargs) = {init};	\
		ucontext_t ol;					\
		ucontext_t nu;					\
		union trampint_u ap = {tr};			\
								\
		if (getcontext(&nu) < 0) {			\
			abort();				\
		}						\
								\
		TMP(res).ssz = 64U * 1024U;			\
		TMP(res).stk = malloc(TMP(res).ssz);		\
								\
		nu.uc_link = &ol;				\
		nu.uc_stack.ss_sp = TMP(res).stk;		\
		nu.uc_stack.ss_size = TMP(res).ssz;		\
		nu.uc_stack.ss_flags = 0;			\
								\
		tr->c = &TMP(res);				\
		tr->caller = &__##x##trampo;			\
		tr->action = (const void*(*)())&x;		\
		tr->initargs = &TMP(initargs);			\
								\
		makecontext(					\
			&nu,					\
			(void(*)())trampoline,			\
			sizeof(ap) / sizeof(int),		\
			ap.i[0], ap.i[1], ap.i[2], ap.i[3]);	\
								\
		if (!_setjmp(__##x##trampo)) {			\
			swapcontext(&ol, &nu);			\
		}						\
		TMP(res);					\
	})

#define free_coru(x)				\
	({					\
		if (x.stk != NULL) {		\
			free(x.stk);		\
		}				\
		x.stk = NULL;			\
		x.ssz = 0U;			\
	})

#define yield(yld)				\
	({					\
		coru_t TMP(tmp) = ____caller;	\
		yield_to(TMP(tmp), yld);	\
	})

#define yield_to(x, yld)					\
	({							\
		static typeof((yld)) TMP(res);			\
								\
		TMP(res) = yld;					\
		____glob = (intptr_t)&TMP(res);			\
		if (!_setjmp(*____callee.jb)) {			\
			char *TMP(brth) = alloca(0x2000U);	\
			asm volatile ("" :: "m" (TMP(brth)));	\
			____caller = ____callee;		\
			____callee = x;				\
			_longjmp(*x.jb, 1);			\
		}						\
		(void*)____glob;				\
	})

#define next(x)			next_with(x, NULL)

#define next_with(x, val)				\
	({						\
		static jmp_buf __##x##sb;		\
		static typeof((val)) TMP(res);		\
							\
		TMP(res) = val;				\
		____glob = (intptr_t)&TMP(res);		\
		if (!_setjmp(__##x##sb)) {		\
			____caller.jb = &__##x##sb;	\
			____callee = x;			\
			_longjmp(*x.jb, 1);		\
		}					\
		(void*)____glob;			\
	})

#endif	/* USE_ASM_CORUS */

#endif	/* INCLUDED_coru_h_ */
