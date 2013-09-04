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
	int res = 0;

	/* output things */
	ctl_caev_pr(dvca[0]);
	ctl_caev_pr(dvca[1]);
	ctl_caev_pr(sum);

	/* check things programmatically */
	if (sum.mktprc.r.p || sum.mktprc.r.q) {
		/* r cell should be null_ratio */
		res = 1;
	}
	if (sum.mktprc.a != -3.50df) {
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

/* caev.01.c ends here */
