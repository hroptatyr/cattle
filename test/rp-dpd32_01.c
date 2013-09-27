#include <stdlib.h>
#include <stdio.h>
#include "ctl-dfp754.h"
#include "nifty.h"

int
main(int argc, char *argv[])
{
/* simple read-print util */

	for (char *const *p = argv + 1U; *p; p++) {
		char *on;
		_Decimal32 x = strtodpd32(*p, &on);

		if (!*on) {
			char buf[64U];
			size_t len = dpd32tostr(buf, sizeof(buf), x);

			if (len < sizeof(buf)) {
				buf[len] = '\0';
				puts(buf);
			}
		}
	}
	return 0;
}

/* rp-dpd32_01.c ends here */
