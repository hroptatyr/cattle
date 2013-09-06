#include "caev-io.c"
#include "caev-supp.h"

int
main(void)
{
	ctl_fund_t f = {.mktprc = 17.50df, .nomval = 1.df, .outsec = 1000.df};
	ctl_caev_t d = make_drip((ctl_ratio_t){1, 9});
	ctl_fund_t new = ctl_caev_act(d, f);
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	ctl_caev_pr(d);
	ctl_fund_pr(new);
	return res;
}

/* caev-supp.04.c ends here */
