#include "caev-io.c"

int
main(void)
{
	ctl_caev_t bids[] = {{
			.outsec.a = -200000,
		}, {
			.outsec.a = +150000,
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
	if (f.outsec != (175 - 20 + 15) * 10000) {
		res = 1;
	}
	return res;
}

/* caev_qact.01.c ends here */
