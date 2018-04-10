#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdio.h>
#include "dfp754_d32.h"
#include "nifty.h"

int
main(void)
{
	_Decimal32 x[] = {
		0.df, 1.df, 1.23df, 0.23df, 1.234df, 12.34df,
		12.df, 100.df, 123.df,
	};

	for (size_t i = 0U; i < countof(x); i++) {
		char buf[64U];
		int l = d32tostr(buf, sizeof(buf), x[i]);
		buf[l] = '\0';
		puts(buf);
	}
	return 0;
}
