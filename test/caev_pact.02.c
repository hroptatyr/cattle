#include "caev-io.c"

int
main(void)
{
	ctl_caev_t splf[] = {{
			.mktprc.r = (ctl_ratio_t){1, 10},
		}, {
			.mktprc.r = (ctl_ratio_t){2, 5},
		}};
	ctl_fund_t f = {
		.mktprc = 17.50df,
	};
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	f = ctl_caev_act(splf[0], f);
	ctl_fund_pr(f);
	f = ctl_caev_act(splf[1], f);
	ctl_fund_pr(f);

	/* check things programmatically */
	if (f.nomval || f.outsec) {
		res = 1;
	}
	if (f.mktprc != ((17.50df / 10.df) * 2.df) / 5.df) {
		res = 1;
	}
	return res;
}

/* caev_pact.02.c ends here */
