/*** caev=ctl1.c -- cattle canonical format, version 1
 *
 * Copyright (C) 2013-2018 Sebastian Freundt
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
#include "cattle.h"
#include "caev.h"
#include "caev-supp.h"

ctl_caev_t
make_ctl1(const ctl_fld_t f[static 1], size_t nf)
{
	ctl_caev_t res = ctl_zero_caev();

	WITH_CTL_FLD(ctl_custm_t mkt, CTL_FLD_XMKT, f, nf, custm) {
		res.mktprc.r = mkt.r;
		res.mktprc.a = mkt.a;
	}
	WITH_CTL_FLD(ctl_custm_t nom, CTL_FLD_XNOM, f, nf, custm) {
		res.nomval.r = nom.r;
		res.nomval.a = nom.a;
	}
	WITH_CTL_FLD(ctl_custm_t out, CTL_FLD_XOUT, f, nf, custm) {
		res.outsec.r = out.r;
		res.outsec.a = out.a;
	}
	return res;
}

/* caev=ctl1.c ends here*/
