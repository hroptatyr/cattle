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
	int res = 0;

	/* output things */
	ctl_caev_pr(splf[0]);
	ctl_caev_pr(splf[1]);
	ctl_caev_pr(sum);

	/* check things programmatically */
	if (sum.mktprc.a) {
		/* a cell should be 0.df */
		res = 1;
	}
	if (sum.mktprc.r.p != 25 || sum.mktprc.r.q != 1U) {
		res = 1;
	}
	if (sum.nomval.r.p || sum.nomval.r.q || sum.nomval.a) {
		res = 1;
	}
	if (sum.outsec.r.p || sum.outsec.r.q || sum.outsec.a) {
		res = 1;
	}
	return res;
}

/* caev.02.c ends here */
