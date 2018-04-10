#include <stdlib.h>
#include <stdio.h>
#include "dfp754_d32.h"
#include "nifty.h"

int
main(int argc, char *argv[])
{
/* simple read-print util */

	for (char *const *p = argv + 1U; *p; p++) {
		char *on;
		_Decimal32 x = strtobid32(*p, &on);

		if (!*on) {
			char buf[64U];
			size_t len = bid32tostr(buf, sizeof(buf), x);

			if (len < sizeof(buf)) {
				buf[len] = '\0';
				puts(buf);
			}
		}
	}
	return 0;
}

/* rp-bid32_01.c ends here */
