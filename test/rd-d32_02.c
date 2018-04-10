#if defined HAVE_CONFIG_H
# include "config.h"
#endif	/* HAVE_CONFIG_H */
#include <stdlib.h>
#include <stdio.h>
#include "dfp754_d32.h"
#include "nifty.h"
#if defined HAVE_DFP754_H
# include <dfp754.h>
#endif	/* HAVE_DFP754_H */

#if !defined _Static_assert
# define _Static_assert(cond, msg)
#endif	/* !_Static_assert */
#define static_assert(x)	_Static_assert(x, paste("in line ", __LINE__))

int
main(void)
{
	static const char *src[] = {
		"0", "1", "1.23", "0.23", "1.234", "12.34",
		"12", "100", "123",
		"9898", "989.8", "98.98", "9.898", ".9898",
	};
	_Decimal32 ref[] = {
		0.df, 1.df, 1.23df, 0.23df, 1.234df, 12.34df,
		12.df, 100.df, 123.df,
		9898.df, 989.8df, 98.98df, 9.898df, .9898df,
	};
	int res = 0;

#if defined HAVE_DFP754_H
	static_assert(countof(src) == countof(ref));
	for (size_t i = 0U; i < countof(src); i++) {
		char *on;
		_Decimal32 x = strtodpd32(src[i], &on);
		_Decimal32 r = decimal32_to_dpd32(ref[i]);

		if (x != r) {
			printf("%x v %x\n", bits(r), bits(x));
			res = 1;
		}
	}
#endif	/* HAVE_DFP754_H */
	return res;
}

/* rd-d32_02.c ends here */
