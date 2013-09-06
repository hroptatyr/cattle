#include <string.h>
#include "caev-io.c"

int
main(void)
{
	ctl_caev_t splf = {
		.mktprc.r = (ctl_ratio_t){10, 1},
	};
	ctl_caev_t rev = ctl_caev_rev(splf);
	ctl_caev_t r2v = ctl_caev_rev(rev);
	int res = 0;

	/* output things */
	ctl_caev_pr(splf);
	ctl_caev_pr(rev);
	ctl_caev_pr(r2v);

	/* unroll more, so we dont suffer from initialisation problems */
	rev = ctl_caev_rev(r2v);
	splf = ctl_caev_rev(rev);

	/* rev should be an involution */
	if (memcmp(&splf, &r2v, sizeof(splf))) {
		ctl_aux_hexpr(&splf, sizeof(splf));
		ctl_aux_hexpr(&r2v, sizeof(r2v));
		res = 1;
	}
	return res;
}

/* caev.06.c ends here */
