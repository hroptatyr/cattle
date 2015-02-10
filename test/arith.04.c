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
/* testing inverses */
	ctl_caev_t _0 = {0U};
	ctl_caev_t _m = {{.r = {1, 2}}};
	ctl_caev_t _a = {{.a = -2.df}};
	ctl_caev_t _b = {{.a = -1.df, .r = {1, 2}}};
	ctl_caev_t r;
	int rc = 0;

	r = ctl_caev_sub(_0, _a);
	CHECK_EQUAL(ctl_caev_inv(_a), r);

	r = ctl_caev_sup(_0, _a);
	CHECK_EQUAL(ctl_caev_inv(_a), r);

	r = ctl_caev_sub(_0, _m);
	CHECK_EQUAL(ctl_caev_inv(_m), r);

	r = ctl_caev_sup(_0, _m);
	CHECK_EQUAL(ctl_caev_inv(_m), r);

	r = ctl_caev_sub(_0, _b);
	CHECK_EQUAL(ctl_caev_inv(_b), r);

	r = ctl_caev_sup(_0, _b);
	CHECK_EQUAL(ctl_caev_inv(_b), r);

	return rc;
}

/* arith.04.c ends here */
