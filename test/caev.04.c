#include <string.h>
#include "caev-io.c"

int
main(void)
{
	ctl_caev_t splf[] = {{
			.mktprc.r = (ctl_ratio_t){10, 1},
		}, {
			.mktprc.r = (ctl_ratio_t){5, 2},
		}};
	ctl_caev_t sum = ctl_caev_add(splf[0], splf[1]);
	ctl_caev_t dif = ctl_caev_sub(sum, splf[1]);
	int res = 0;

	/* output things */
	ctl_caev_pr(splf[0]);
	ctl_caev_pr(splf[1]);
	ctl_caev_pr(sum);
	ctl_caev_pr(dif);

	/* check things programmatically */
	if (memcmp(&dif, splf, sizeof(dif))) {
		res = 1;
	}
	return res;
}

/* caev.04.c ends here */
