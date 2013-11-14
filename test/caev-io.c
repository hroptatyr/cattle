/* just some routines for caev_t printing */
#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdint.h>
#include <stdio.h>
#include "cattle-ratio.h"
#include "cattle-price.h"
#include "cattle-quant.h"
#include "cattle-perio.h"
#include "cattle-date.h"
#include "caev.h"
#include "ctl-dfp754.h"

static inline void
ctl_caev_pr(ctl_caev_t e)
{
	char buf[32U];

	bid32tostr(buf, sizeof(buf), e.mktprc.a);
	printf("m: %d<-%u + %s  ", e.mktprc.r.p, e.mktprc.r.q, buf);
	bid32tostr(buf, sizeof(buf), e.nomval.a);
	printf("n: %d<-%u + %s  ", e.nomval.r.p, e.nomval.r.q, buf);
	bid32tostr(buf, sizeof(buf), e.outsec.a);
	printf("o: %d<-%u + %s\n", e.outsec.r.p, e.outsec.r.q, buf);
	return;
}

static inline void
ctl_fund_pr(ctl_fund_t f)
{
	char buf[32U];

	bid32tostr(buf, sizeof(buf), f.mktprc);
	printf("m: %s  ", buf);
	bid32tostr(buf, sizeof(buf), f.nomval);
	printf("n: %s  ", buf);
	bid32tostr(buf, sizeof(buf), f.outsec);
	printf("o: %s\n", buf);
	return;
}

static __attribute__((unused)) void
ctl_aux_hexpr(const void *buf, size_t bsz)
{
	static char hx[] = "0123456789abcdef";
	for (const uint8_t *p = buf, *const ep = p + bsz; p < ep; p++) {
		putchar(hx[(*p & 0xf0U) >> 4U]);
		putchar(hx[(*p & 0x0fU) >> 0U]);
	}
	putchar('\n');
	return;
}

/* caev-io.c ends here */
