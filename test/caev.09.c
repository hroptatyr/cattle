#include "caev-io.c"

int
main(void)
{
	ctl_caev_t splf[] = {{
			.mktprc.r = (ctl_ratio_t){0, 0},
			.mktprc.a = 1.df,
		}, {
			.mktprc.r = (ctl_ratio_t){5, 2},
			.mktprc.a = 2.df,
		}};
	ctl_caev_t sum = ctl_caev_add(splf[0], splf[1]);
	int res = 0;

	/* output things */
	ctl_caev_pr(splf[0]);
	ctl_caev_pr(splf[1]);
	ctl_caev_pr(sum);
	return res;
}

/* caev.09.c ends here */
