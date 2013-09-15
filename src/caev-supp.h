/*** caev=splf.c -- forward split
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
#if !defined INCLUDED_caev_supp_h_
#define INCLUDED_caev_supp_h_

#include <stdlib.h>
#include "cattle.h"
#include "caev.h"

typedef union {
	ctl_ratio_t r;
	ctl_price_t p;
	ctl_quant_t q;
	ctl_perio_t i;
	ctl_date_t d;
} ctl_fld_val_t;

typedef struct ctl_fld_s ctl_fld_t;

/* unknown fields */
typedef enum {
	CTL_FLD_UNK,
	CTL_FLD_UNK_LAST,
} ctl_fld_unk_t;

struct ctl_fld_s {
	ctl_fld_unk_t code;
	ctl_fld_val_t beef;
};

typedef enum {
	CTL_CAEV_UNK,
	CTL_CAEV_BONU,
	CTL_CAEV_CAPD,
	CTL_CAEV_CAPG,
	CTL_CAEV_DECR,
	CTL_CAEV_DRIP,
	CTL_CAEV_DVCA,
	CTL_CAEV_DVOP,
	CTL_CAEV_DVSC,
	CTL_CAEV_DVSE,
	CTL_CAEV_INCR,
	CTL_CAEV_LIQU,
	CTL_CAEV_RHDI,
	CTL_CAEV_RHTS,
	CTL_CAEV_SPLF,
	CTL_CAEV_SPLR,
} ctl_caev_code_t;

/* date fields */
typedef enum {
	CTL_FLD_DATE_FIRST = CTL_FLD_UNK_LAST + 1U,
	CTL_FLD_ANOU = CTL_FLD_DATE_FIRST,
	CTL_FLD_AVAL,
	CTL_FLD_CERT,
	CTL_FLD_COAP,
	CTL_FLD_CVPR,
	CTL_FLD_DVCP,
	CTL_FLD_DIVR,
	CTL_FLD_EARD,
	CTL_FLD_EARL,
	CTL_FLD_ECDT,
	CTL_FLD_ECPD,
	CTL_FLD_EFFD,
	CTL_FLD_EQUL,
	CTL_FLD_ETPD,
	CTL_FLD_EXPI,
	CTL_FLD_FDAT,
	CTL_FLD_FILL,
	CTL_FLD_FXDT,
	CTL_FLD_GUPA,
	CTL_FLD_HEAR,
	CTL_FLD_IFIX,
	CTL_FLD_LAPD,
	CTL_FLD_LOTO,
	CTL_FLD_LTRD,
	CTL_FLD_MATU,
	CTL_FLD_MCTD,
	CTL_FLD_MEET,
	CTL_FLD_MET2,
	CTL_FLD_MET3,
	CTL_FLD_MKDT,
	CTL_FLD_MFIX,
	CTL_FLD_OAPD,
	CTL_FLD_PAYD,
	CTL_FLD_PLDT,
	CTL_FLD_PODT,
	CTL_FLD_POST,
	CTL_FLD_PROD,
	CTL_FLD_PPDT,
	CTL_FLD_RDDT,
	CTL_FLD_RDTE,
	CTL_FLD_REGI,
	CTL_FLD_RESU,
	CTL_FLD_SPLT,
	CTL_FLD_SUBS,
	CTL_FLD_SXDT,
	CTL_FLD_TAXB,
	CTL_FLD_TRAD,
	CTL_FLD_TSDT,
	CTL_FLD_TPDT,
	CTL_FLD_UNCO,
	CTL_FLD_WUCO,
	CTL_FLD_VALU,
	CTL_FLD_XDTE,
	CTL_FLD_DATE_LAST,
} ctl_fld_date_t;

/* ratio fields */
typedef enum {
	CTL_FLD_RATIO_FIRST = CTL_FLD_DATE_LAST + 1U,
	CTL_FLD_ADEX = CTL_FLD_RATIO_FIRST,
	CTL_FLD_ADSR,
	CTL_FLD_NEWO,
	CTL_FLD_RTUN,
	CTL_FLD_RATIO_LAST,
} ctl_fld_ratio_t;

/* price fields */
typedef enum {
	CTL_FLD_PRICE_FIRST = CTL_FLD_RATIO_LAST + 1U,
	CTL_FLD_ATAX = CTL_FLD_PRICE_FIRST,
	CTL_FLD_BIDI,
	CTL_FLD_CHAR,
	CTL_FLD_DEVI,
	CTL_FLD_EQUR/*actually EQUL*/,
	CTL_FLD_ESOF,
	CTL_FLD_EXCH,
	CTL_FLD_FDIV,
	CTL_FLD_FISC,
	CTL_FLD_FLFR,
	CTL_FLD_GRSS,
	CTL_FLD_IDFX,
	CTL_FLD_INCE,
	CTL_FLD_INDX,
	CTL_FLD_INTP,
	CTL_FLD_INTR,
	CTL_FLD_NETT,
	CTL_FLD_NRES,
	CTL_FLD_NWFC,
	CTL_FLD_OVEP,
	CTL_FLD_PDIV,
	CTL_FLD_PRFC,
	CTL_FLD_PROR,
	CTL_FLD_PTSC,
	CTL_FLD_RATE,
	CTL_FLD_RDIS,
	CTL_FLD_RINR,
	CTL_FLD_RLOS,
	CTL_FLD_RSPR,
	CTL_FLD_SHRT,
	CTL_FLD_SOFE,
	CTL_FLD_TAXC,
	CTL_FLD_TAXE,
	CTL_FLD_TAXR,
	CTL_FLD_TDMT,
	CTL_FLD_TRAT,
	CTL_FLD_TXIN,
	CTL_FLD_TXPR,
	CTL_FLD_TXRC,
	CTL_FLD_WAPA,
	CTL_FLD_WITF,
	CTL_FLD_WITL,
	/* from here on what swift considers a price */
	CTL_FLD_CAVA,
	CTL_FLD_CINL,
	CTL_FLD_INDC,
	CTL_FLD_ISSU,
	CTL_FLD_MAXP,
	CTL_FLD_MINP,
	CTL_FLD_MRKT,
	CTL_FLD_OFFR,
	CTL_FLD_OSUB,
	CTL_FLD_PRPP,
	CTL_FLD_PRICE_LAST,
} ctl_fld_price_t;

/* period fields */
typedef enum {
	CTL_FLD_PERIOD_FIRST = CTL_FLD_PRICE_LAST + 1U,
	CTL_FLD_AREV = CTL_FLD_PERIOD_FIRST,
	CTL_FLD_BLOK,
	CTL_FLD_BOCL,
	CTL_FLD_CLCP,
	CTL_FLD_CODS,
	CTL_FLD_CSPD,
	CTL_FLD_DSBT,
	CTL_FLD_DSDA,
	CTL_FLD_DSDE,
	CTL_FLD_DSPL,
	CTL_FLD_DSSE,
	CTL_FLD_DSWA,
	CTL_FLD_DSWN,
	CTL_FLD_DSWO,
	CTL_FLD_DSWS,
	CTL_FLD_INPE,
	CTL_FLD_PARL,
	CTL_FLD_PRIC,
	CTL_FLD_PWAL,
	CTL_FLD_REVO,
	CTL_FLD_SUSP,
	CTL_FLD_TRDP,
	CTL_FLD_PERIOD_LAST,
} ctl_fld_period_t;

typedef enum {
	CTL_FLD_TYPE_UNK,
	CTL_FLD_TYPE_DATE,
	CTL_FLD_TYPE_RATIO,
	CTL_FLD_TYPE_PRICE,
	CTL_FLD_TYPE_PERIOD,
} ctl_fld_type_t;


/* the following use SMPG_CA_Global_Market_Practice_Part_2_SR2013_v_1_2
 * grid values as their parameters. */
/**
 * Return bonus distribution event based on pro-rata assignments. */
extern ctl_caev_t make_bonu(const ctl_fld_t f[static 1], size_t n);

#if 0
/**
 * Return capital from the capital account. */
extern ctl_caev_t make_capd(const ctl_fld_t f[static 1], size_t n);

/**
 * Return capital gains from the capital account. */
extern ctl_caev_t make_capg(const ctl_fld_t f[static 1], size_t n);
#endif	/* 0 */

/**
 * Return dividend reinvestment event based on additional securities. */
extern ctl_caev_t make_drip(const ctl_fld_t f[static 1], size_t n);

/**
 * Return cash dividend event based on net price per security. */
extern ctl_caev_t make_dvca(const ctl_fld_t f[static 1], size_t n);

/**
 * Return stock dividend event based on pro-rata assignments. */
extern ctl_caev_t make_dvse(const ctl_fld_t f[static 1], size_t n);

/**
 * Return forward split event based on new-for-old ratio. */
extern ctl_caev_t make_splf(const ctl_fld_t f[static 1], size_t n);

/**
 * Return reverse split event based on new-for-old ratio. */
extern ctl_caev_t make_splr(const ctl_fld_t f[static 1], size_t n);


/* helpers */
static inline __attribute__((pure, const)) ctl_fld_type_t
ctl_fld_type(ctl_fld_unk_t code)
{
	switch (code) {
	case CTL_FLD_DATE_FIRST ... CTL_FLD_DATE_LAST:
		return CTL_FLD_TYPE_DATE;
	case CTL_FLD_RATIO_FIRST ... CTL_FLD_RATIO_LAST:;
		return CTL_FLD_TYPE_RATIO;
	case CTL_FLD_PRICE_FIRST ... CTL_FLD_PRICE_LAST:
		return CTL_FLD_TYPE_PRICE;
	case CTL_FLD_PERIOD_FIRST ... CTL_FLD_PERIOD_LAST:
		return CTL_FLD_TYPE_PERIOD;
	default:
		break;
	}
	return CTL_FLD_TYPE_UNK;
}

static inline __attribute__((pure)) ctl_fld_t
ctl_find_fld(const ctl_fld_t f[static 1], size_t n, ctl_fld_unk_t fld)
{
	for (size_t i = 0; i < n; i++) {
		if (f[i].code == fld) {
			return f[i];
		}
	}
	return (ctl_fld_t){CTL_FLD_UNK};
}

#endif	/* INCLUDED_caev_supp_h_ */
