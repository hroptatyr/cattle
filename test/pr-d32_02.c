#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdio.h>
#include "ctl-dfp754.h"
#include "nifty.h"
#if defined HAVE_DFP754_H
# include <dfp754.h>
#endif	/* HAVE_DFP754_H */

int
main(void)
{
	_Decimal32 x[] = {
		0.df, 1.df, 1.23df, 0.23df, 1.234df, 12.34df,
		12.df, 100.df, 123.df,
		9.898df, 98.98df, 0.9898df, 989.8df,
		987321.0df, 123456.7df, 0.00888df
	};

#if defined HAVE_DFP754_H
	for (size_t i = 0U; i < countof(x); i++) {
		char buf[64U];
		_Decimal32 r = decimal32_to_dpd32(x[i]);
		int l = dpd32tostr(buf, sizeof(buf), r);
		buf[l] = '\0';
		puts(buf);
	}
#endif	/* HAVE_DFP754_H */
	return 0;
}

/* pr-d32_02.c ends here */
