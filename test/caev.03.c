#include <string.h>
#include "caev-io.c"

int
main(void)
{
	ctl_caev_t dvca[] = {{
			.mktprc.a = -2.00df,
		}, {
			.mktprc.a = -1.50df,
		}};
	ctl_caev_t sum = ctl_caev_add(dvca[0], dvca[1]);
	ctl_caev_t dif = ctl_caev_sub(sum, dvca[1]);
	int res = 0;

	/* output things */
	ctl_caev_pr(dvca[0]);
	ctl_caev_pr(dvca[1]);
	ctl_caev_pr(sum);
	ctl_caev_pr(dif);

	/* check things programmatically */
	if (memcmp(&dif, dvca, sizeof(dif))) {
		res = 1;
	}
	return res;
}

/* caev.03.c ends here */
