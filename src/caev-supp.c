/*** caev-supp.c -- supported message fields and messages
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
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include "cattle.h"
#include "caev.h"
#include "caev-supp.h"


ctl_caev_t
make_caev(const ctl_fld_t msg[static 1], size_t nflds)
{
	WITH_CTL_FLD(ctl_caev_code_t caev, CTL_FLD_CAEV, msg, nflds, caev) {
		switch (caev) {
		case CTL_CAEV_BONU:
			return make_bonu(msg, nflds);
		case CTL_CAEV_CAPD:
			break;
		case CTL_CAEV_CAPG:
			break;
		case CTL_CAEV_DECR:
			break;
		case CTL_CAEV_DRIP:
			return make_drip(msg, nflds);
		case CTL_CAEV_DVCA:
			return make_dvca(msg, nflds);
		case CTL_CAEV_DVOP:
			break;
		case CTL_CAEV_DVSC:
			break;
		case CTL_CAEV_DVSE:
			return make_dvse(msg, nflds);
		case CTL_CAEV_INCR:
			break;
		case CTL_CAEV_LIQU:
			break;
		case CTL_CAEV_RHDI:
			break;
		case CTL_CAEV_RHTS:
			break;
		case CTL_CAEV_SPLF:
			return make_splf(msg, nflds);
		case CTL_CAEV_SPLR:
			return make_splr(msg, nflds);
		default:
			break;
		}
	}
	return (ctl_caev_t){};
}

/* caev-supp.c ends here */
