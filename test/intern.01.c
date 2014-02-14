#include <stdio.h>
#include "intern.h"

int
main(void)
{
	const char *x;
	int rc = 0;

	printf("%p %s\n", intern("FOO", 3), obint_name(intern("FOO", 3)));
	printf("%p %s\n", intern("BAR", 3), obint_name(intern("BAR", 3)));
	printf("%p %s\n", intern("FOO", 3), obint_name(intern("FOO", 3)));

	rc |= intern("FOO", 3) != intern("FOO", 3);
	rc |= intern("BAR", 3) != intern("BAR", 3);
	clear_interns();
	return rc;
}

/* intern.01.c ends here */
