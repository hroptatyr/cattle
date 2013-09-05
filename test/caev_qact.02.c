#include "caev-io.c"

int
main(void)
{
	ctl_caev_t bids[] = {{
			.outsec.r = (ctl_ratio_t){2, 1},
		}, {
			.outsec.r = (ctl_ratio_t){2, 5},
		}};
	ctl_fund_t f = {
		.outsec = 1750000,
	};
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	f = ctl_caev_act(bids[0], f);
	ctl_fund_pr(f);
	f = ctl_caev_act(bids[1], f);
	ctl_fund_pr(f);

	/* check things programmatically */
	if (f.mktprc || f.nomval) {
		res = 1;
	}
	if (f.outsec != 1750000 * 2 * 2 / 5) {
		res = 1;
	}
	return res;
}

/* caev_qact.02.c ends here */
