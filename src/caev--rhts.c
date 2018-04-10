/*** caev=rhts.c -- rights distribution
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
make_rhts(const ctl_fld_t f[static 1], size_t nf)
{
/* return the event actor for rtun (rights-to-undly ratio) */
	ctl_caev_t res = ctl_zero_caev();

	WITH_CTL_FLD(ctl_ratio_t rtun, CTL_FLD_RTUN, f, nf, ratio) {
		/* the actual formula is (pP + qQ)/(p+q) but we have
		 * to represent it as p/(p+q) for the multiplicative bit
		 * and qQ/(p+q) for the additive bit.
		 * P is the market price, Q the rights price, for a
		 * rights-to-underlying ratio of q/p. */
		res.mktprc.r = ctl_ratio_recipr(ctl_adex_to_newo(rtun));
		res.outsec.r = ctl_adex_to_newo(rtun);
		res.nomval.r = ctl_ratio_recipr(ctl_adex_to_newo(rtun));

		WITH_CTL_FLD(ctl_price_t prpp, CTL_FLD_PRPP, f, nf, price) {
			res.mktprc.a = prpp * rtun.p;
			res.nomval.a = 0.df;
			res.outsec.a = 0.df;
		}
	}
	return res;
}

/* caev=rhts.c ends here*/
