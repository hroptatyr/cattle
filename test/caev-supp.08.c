#include "caev-io.c"
#include "caev-supp.h"
#include "nifty.h"

int
main(void)
{
	ctl_fund_t f = {.mktprc = 17.50df, .nomval = 1.df, .outsec = 1000.df};
	ctl_fld_t msg[] = {
		MAKE_CTL_FLD(admin, CTL_FLD_CAEV, CTL_CAEV_SPLF),
		MAKE_CTL_FLD(ratio, CTL_FLD_NEWO, {10, 1U}),
	};
	ctl_caev_t d = make_caev(msg, countof(msg));
	ctl_fund_t new = ctl_caev_act(d, f);
	int res = 0;

	/* output things */
	ctl_fund_pr(f);
	ctl_caev_pr(d);
	ctl_fund_pr(new);
	return res;
}

/* caev-supp.08.c ends here */
