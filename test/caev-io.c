/* just some routines for caev_t printing */
#include <stdint.h>
#include <stdio.h>
#include "cattle.h"
#include "caev.h"

static inline void
ctl_caev_pr(ctl_caev_t e)
{
	printf("m: %d<-%u + %f  ", e.mktprc.r.p, e.mktprc.r.q, (float)e.mktprc.a);
	printf("n: %d<-%u + %f  ", e.nomval.r.p, e.nomval.r.q, (float)e.nomval.a);
	printf("o: %d<-%u + %f\n", e.outsec.r.p, e.outsec.r.q, (float)e.outsec.a);
	return;
}

static inline void
ctl_fund_pr(ctl_fund_t f)
{
	printf("m: %f  ", (float)f.mktprc);
	printf("n: %f  ", (float)f.nomval);
	printf("o: %f\n", (float)f.outsec);
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
