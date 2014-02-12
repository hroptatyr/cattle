#include <stdio.h>
#include "intern.h"

int
main(void)
{
	const char *x;
	int rc = 0;

	printf("%p %s\n", intern("FOO"), intern("FOO"));
	printf("%p %s\n", intern("BAR"), intern("BAR"));
	printf("%p %s\n", intern("FOO"), intern("FOO"));

	rc |= intern("FOO") != intern("FOO");
	rc |= intern("BAR") != intern("BAR");
	clear_interns();
	return rc;
}

/* intern.01.c ends here */
