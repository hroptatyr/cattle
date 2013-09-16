#include "caev-io.c"
#include "caev-supp.h"
#include "nifty.h"

int
main(void)
{
	ctl_fund_t f = {.mktprc = 17.50df, .nomval = 1.df, .outsec = 1000.df};
	ctl_fld_t msg[] = {{
			.code = CTL_FLD_ADEX,
			.beef.r = {1, 9U},
		}};
	ctl_caev_t d = make_drip(msg, countof(msg));
	ctl_fund_t new = ctl_caev_act(d, f);
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	ctl_caev_pr(d);
	ctl_fund_pr(new);
	return res;
}

/* caev-supp.04.c ends here */
