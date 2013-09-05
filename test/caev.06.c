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

	/* rev should be an involution */
	if (memcmp(&splf, &r2v, sizeof(splf))) {
		res = 1;
	}
	return res;
}

/* caev.06.c ends here */
