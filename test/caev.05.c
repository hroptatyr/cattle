#include <string.h>
#include "caev-io.c"

int
main(void)
{
	ctl_caev_t dvca = {
		.mktprc.a = -2.00df,
	};
	ctl_caev_t rev = ctl_caev_rev(dvca);
	ctl_caev_t r2v = ctl_caev_rev(rev);
	int res = 0;

	/* output things */
	ctl_caev_pr(dvca);
	ctl_caev_pr(rev);
	ctl_caev_pr(r2v);

	/* unroll more, so we dont suffer from initialisation problems */
	rev = ctl_caev_rev(r2v);
	dvca = ctl_caev_rev(rev);

	/* we use the fact that rev is an involution */
	if (memcmp(&dvca, &r2v, sizeof(dvca))) {
		ctl_aux_hexpr(&dvca, sizeof(dvca));
		ctl_aux_hexpr(&r2v, sizeof(r2v));
		res = 1;
	}
	return res;
}

/* caev.05.c ends here */
