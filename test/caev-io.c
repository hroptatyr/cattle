/* just some routines for caev_t printing */
#include <stdio.h>
#include "cattle.h"
#include "caev.h"

static void
ctl_caev_pr(ctl_caev_t e)
{
	printf("m: %d<-%u + %f  ", e.mktprc.r.p, e.mktprc.r.q, (float)e.mktprc.a);
	printf("n: %d<-%u + %f  ", e.nomval.r.p, e.nomval.r.q, (float)e.nomval.a);
	printf("o: %d<-%u + %f\n", e.outsec.r.p, e.outsec.r.q, (float)e.outsec.a);
	return;
}

/* caev-io.c ends here */
