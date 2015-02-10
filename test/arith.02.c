#include <stdbool.h>
#include "caev-io.c"
#include "caev-supp.h"
#include "nifty.h"

#define CHECK_EQUAL(_want, _have)		\
	if (!ctl_caev_equal_p(_want, _have)) {	\
		fputs("wanted\t", stdout);	\
		ctl_caev_pr(_want);		\
		fputs("got\t", stdout);		\
		ctl_caev_pr(_have);		\
		rc++;				\
	}

int
main(void)
{
/* testing neutral element */
	ctl_caev_t _0 = {0U};
	ctl_caev_t _1 = {{.r = {1, 1}}};
	ctl_caev_t _m = {{.r = {1, 2}}};
	ctl_caev_t r;
	int rc = 0;

	r = ctl_caev_add(_0, _m);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_m, _0);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_1, _m);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_m, _1);
	CHECK_EQUAL(_m, r);
	return rc;
}

/* arith.02.c ends here */
