#include "caev-io.c"

int
main(void)
{
	ctl_caev_t dvca[] = {{
			.mktprc.a = -2.00df,
		}, {
			.mktprc.a = -1.50df,
		}};
	ctl_fund_t f = {
		.mktprc = 17.50df,
	};
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	f = ctl_caev_act(dvca[0], f);
	ctl_fund_pr(f);
	f = ctl_caev_act(dvca[1], f);
	ctl_fund_pr(f);

	/* check things programmatically */
	if (f.nomval || f.outsec) {
		res = 1;
	}
	if (f.mktprc != 17.50df + -2.00df + -1.50df) {
		res = 1;
	}
	return res;
}

/* caev_pact.01.c ends here */
