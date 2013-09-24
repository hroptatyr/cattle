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
		{1992, 1, 3, ECHS_ALL_DAY},
		{1991, 2, 2, ECHS_ALL_DAY},
		{1990, 3, 1, ECHS_ALL_DAY},
		{1994, 4, 4, ECHS_ALL_DAY},
		{1998, 4, 8, ECHS_ALL_DAY},
		{1997, 4, 7, ECHS_ALL_DAY},
		{1996, 4, 6, ECHS_ALL_DAY},
		{1995, 4, 5, ECHS_ALL_DAY},
		{1999, 4, 9, ECHS_ALL_DAY},
		{2001, 4, 11, ECHS_ALL_DAY},
		{2002, 4, 12, ECHS_ALL_DAY},
		{2003, 4, 13, ECHS_ALL_DAY},
		{2000, 4, 10, ECHS_ALL_DAY},
	};
	long unsigned int niter = 3U;
	long unsigned int nmodu = countof(v);
	ctl_wheap_t x = make_ctl_wheap();

	if (argc > 1) {
		niter = strtoul(argv[1U], NULL, 0);
	}
	if (argc > 2) {
		long unsigned int tmp = strtoul(argv[2U], NULL, 0);
		if (tmp <= countof(v)) {
			nmodu = tmp;
		}
	}

	for (size_t j = 0U, i = 0U; j < niter; j++) {
		ctl_wheap_add_deferred(x, v[i], 0U);

		if (i = (i + 1U) >= nmodu) {
			ctl_wheap_fix_deferred(x);
			i = 0;
		}
	}

	for (echs_instant_t t; !__inst_0_p(t = ctl_wheap_top_rank(x));) {
		(void)ctl_wheap_pop(x);

		pr(t);
	}

	free_ctl_wheap(x);
	return 0;
}
