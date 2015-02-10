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
/* testing subtractions */
	ctl_caev_t _0 = {0U};
	ctl_caev_t _1 = {{.r = {1, 1}}};
	ctl_caev_t _m = {{.r = {1, 2}}};
	ctl_caev_t _a = {{.a = -2.df}};
	ctl_caev_t _b = {{.a = -1.df, .r = {1, 2}}};
	ctl_caev_t r;
	int rc = 0;

	CHECK_EQUAL(_0, _1);

	/* adding primitives */
	r = ctl_caev_add(_m, _a);
	r = ctl_caev_sub(r, _a);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_a, _m);
	r = ctl_caev_sub(r, _m);
	CHECK_EQUAL(_a, r);

	r = ctl_caev_add(_m, _a);
	r = ctl_caev_sup(r, _m);
	CHECK_EQUAL(_a, r);

	r = ctl_caev_add(_a, _m);
	r = ctl_caev_sup(r, _a);
	CHECK_EQUAL(_m, r);

	/* adding primitives to mixeds */
	r = ctl_caev_add(_m, _b);
	r = ctl_caev_sub(r, _b);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_a, _b);
	r = ctl_caev_sub(r, _b);
	CHECK_EQUAL(_a, r);

	r = ctl_caev_add(_b, _m);
	r = ctl_caev_sup(r, _b);
	CHECK_EQUAL(_m, r);

	r = ctl_caev_add(_b, _a);
	r = ctl_caev_sup(r, _b);
	CHECK_EQUAL(_a, r);

	r = ctl_caev_add(_b, _m);
	r = ctl_caev_sub(r, _m);
	CHECK_EQUAL(_b, r);

	r = ctl_caev_add(_b, _a);
	r = ctl_caev_sub(r, _a);
	CHECK_EQUAL(_b, r);

	r = ctl_caev_add(_m, _b);
	r = ctl_caev_sup(r, _m);
	CHECK_EQUAL(_b, r);

	r = ctl_caev_add(_a, _b);
	r = ctl_caev_sup(r, _a);
	CHECK_EQUAL(_b, r);

	/* adding mixeds and mixeds */
	r = ctl_caev_add(_b, _b);
	r = ctl_caev_sub(r, _b);
	CHECK_EQUAL(_b, r);

	r = ctl_caev_add(_b, _b);
	r = ctl_caev_sup(r, _b);
	CHECK_EQUAL(_b, r);

	return rc;
}

/* arith.03.c ends here */
