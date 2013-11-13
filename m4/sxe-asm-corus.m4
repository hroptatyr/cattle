dnl sxe-asm-corus.m4 --- asm backed coroutines
dnl
dnl Copyright (C) 2005-2013 Sebastian Freundt
dnl
dnl Author: Sebastian Freundt <hroptatyr@sxemacs.org>
dnl
dnl Redistribution and use in source and binary forms, with or without
dnl modification, are permitted provided that the following conditions
dnl are met:
dnl
dnl 1. Redistributions of source code must retain the above copyright
dnl    notice, this list of conditions and the following disclaimer.
dnl
dnl 2. Redistributions in binary form must reproduce the above copyright
dnl    notice, this list of conditions and the following disclaimer in the
dnl    documentation and/or other materials provided with the distribution.
dnl
dnl 3. Neither the name of the author nor the names of any contributors
dnl    may be used to endorse or promote products derived from this
dnl    software without specific prior written permission.
dnl
dnl THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR
dnl IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
dnl WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
dnl DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
dnl FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
dnl CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
dnl SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
dnl BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
dnl WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
dnl OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
dnl IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
dnl
dnl This file is part of cattle.

AC_DEFUN([_SXE_CHECK_ASM_CORUS], [dnl
	AC_MSG_CHECKING([for asm backed coroutines])

	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#if defined(__i386__)

#elif defined(__x86_64__)

#elif defined(__arm__)  &&  defined(__unix__)

#elif defined(__ppc__)  &&  defined(__APPLE__)

#else
# error "no asm backed coroutines on this platform"
#endif
	]])], [
		sxe_cv_feat_asm_corus="yes"
		$1
	], [
		sxe_cv_feat_asm_corus="no"
		$2
	])

	AC_MSG_RESULT([${sxe_cv_feat_asm_corus}])
])dnl _SXE_CHECK_ASM_CORUS

AC_DEFUN([SXE_CHECK_ASM_CORUS], [dnl
	AC_REQUIRE([_SXE_CHECK_ASM_CORUS])

	AC_ARG_ENABLE([asm-backed-coroutines], [
AS_HELP_STRING([--enable-asm-backed-coroutines], [
Provide coroutines through asm-backed frame switching instead
of more portable setjmp/longjmp/ucontext.])
AS_HELP_STRING([], [Default: enable if supported])],
	[enable_asm_corus="${enableval}"],
	[enable_asm_corus="${sxe_cv_feat_asm_corus}"])
	if test "${sxe_cv_feat_asm_corus}" = "no" -a \
		"${enable_asm_corus}" = "yes"; then
		AC_MSG_WARN([
asm backed coroutines on your wishlist but it's already been determined
that they won't work for your platform.  I hope you know what you're
doing.
])
	fi
	use_asm_corus="${enable_asm_corus}"
])dnl SXE_CHECK_ASM_CORUS

dnl sxe-asm-corus.m4 ends here
