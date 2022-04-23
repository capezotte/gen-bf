#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include "lookup.h"

int main() {
	int old_c = 0, first_run = 1, c;

	while ( (c = getchar()) != EOF ) {
		const int x = c-old_c;
		const char *p = lookup[255+x];
		if (first_run) {
			if (abs(x) >= 240) {
				/* use -(256-x) instead */
				p = lookup[255-(256-x)];
			}
			/* avoid tape underflow */
			if (*p == '<') p++;
			/* reserve first cell for multiplication */
			else if (*p == '+' || *p == '-') putchar('>');
			first_run = 0;
		}
		fputs(p, stdout);
		putchar('.');
		old_c = c;
	}
	putchar('\n');
}
