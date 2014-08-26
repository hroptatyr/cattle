#include <stdio.h>
#include "dt-strpf.h"
#include "nifty.h"
#include "wheap.h"

static void
pr(echs_instant_t i)
{
	char buf[256U];
		
	buf[dt_strf(buf, sizeof(buf), i)] = '\0';
	puts(buf);
	return;
}

int
main(int argc, char *argv[])
{
	static echs_instant_t v[] = {
		{1990, 10, 16, ECHS_ALL_DAY},
		{1991, 9, 16, ECHS_ALL_DAY},
		{1993, 1, 20, ECHS_ALL_DAY},
		{1993, 9, 13, ECHS_ALL_DAY},
		{1991, 9, 13, ECHS_ALL_DAY},
		{1992, 4, 01, ECHS_ALL_DAY},
		{1992, 5, 01, ECHS_ALL_DAY},
		{1992, 6, 01, ECHS_ALL_DAY},
		{1992, 7, 01, ECHS_ALL_DAY},
		{1992, 8, 01, ECHS_ALL_DAY},
		{1992, 9, 01, ECHS_ALL_DAY},
		{1992, 9, 02, ECHS_ALL_DAY},
		{1992, 9, 03, ECHS_ALL_DAY},
	};
	long unsigned int niter = argc > 1 ? strtoul(argv[1U], NULL, 0) : 3U;
	ctl_wheap_t x = make_ctl_wheap();

	for (size_t j = 0U, i = 0U; j < niter; j++, i = (i + 1U) % countof(v)) {
		ctl_wheap_add(x, v[i], 0U);
	}

	for (echs_instant_t t;
	     !echs_nul_instant_p(t = ctl_wheap_top_rank(x));) {
		(void)ctl_wheap_pop(x);

		pr(t);
	}
	return 0;
}
