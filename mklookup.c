#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* every single byte prime */
unsigned char primes[] = {
	2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
	73, 79, 83, 89, 97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151,
	157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223, 227, 229, 233,
	239, 241, 251
};

/* util */
char *xstrdup(const char *s) {
	char *ret = (char *)strdup(s);
	if (ret) return ret;
	perror("strdup");
	exit(1);
}
int xasprintf(char **ret, const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(NULL, 0, fmt, ap);
	va_end(ap);
	if (len < 0) {
		perror("xasprintf");
		exit(1);
	}
	(*ret) = calloc(len+1, sizeof(char));
	if (*ret) {
		va_start(ap, fmt);
		vsnprintf(*ret, len+1, fmt, ap);
		va_end(ap);
		return len+1;
	} else {
		exit(1);
	}
}

char *lookup[512];

/* find the factors the give the shortest multiplication representation. */
void factor(int num, int r[2]) {
	int prev_sum = 255, fac1 = 1;
	switch (num) {
		case 0:
			r[0] = 0; r[1] = 1;
			return;
		case 1:
			r[0] = 1; r[1] = 1;
			return;
		default:
			for (int i = 0; i < 128; i++) {
				const unsigned char p = primes[i];

				if (p >= num) {
					r[0] = num;
					r[1] = fac1;
					return;
				}

				while (num % p == 0) {
					fac1 *= p; num /= p;
					if ( (fac1 + num) < prev_sum) {
						prev_sum = fac1 + num;
					} else {
						fac1 /= p; num *= p; // undo!
						r[0] = num; r[1] = fac1;
						return;
					}
				}
			}
	}
}
/* just a bunch of + or */
char *raw_repr(const int n) {
	char ret[512];
	int i = 0;
	while (i < abs(n)) ret[i++] = abs(n) == n ? '+' : '-';
	ret[i++] = 0;
	return xstrdup(ret);
}
/* use factorization */
char *factor_repr(const int n) {
	char ret[512];
	int r[2], i = 0;
	char sign = abs(n) == n ? '+' : '-';

	factor(abs(n), r);
	ret[i++] = '<';
	while (r[0]--)
		ret[i++] = '+';
	ret[i++] = '[';
	ret[i++] = '-';
	ret[i++] = '>';
	while (r[1]--)
		ret[i++] = sign;
	ret[i++] = '<';
	ret[i++] = ']';
	ret[i++] = '>';
	ret[i++] = 0;
	return xstrdup(ret);
}
/* find the three factors for representation that abuses underflow.
	 <--[----->++<]>
	 x´  `-y-´  `z   */
int underflow_repr_c(const int n, int r[3]) {
	int x, y, z;
	/*
		Literal trial and error.
		TODO: learn number theory
	*/
	for (y = 1; y <= 5; y++) {
		for (x = 1; x <= y; x++) {
			for (z = 1; z <= y; z++) {
				int k = x, v = 0;
				/* will never succeed if n can't be divided by z. */
				if (n % z) continue;
				/* odd + even always odd, will never reach 256. */
				if (x % 2 != y % 2) continue;
				/* will circle around the initial value of k. */
				if ( (256-x)%y == x) continue;
				while (k % 256) {
					k += y;
					v += z;
				}
				v %= 256;
				if (v == n) {
					r[0] = x;
					r[1] = y;
					r[2] = z;
					return 0;
				}
			}
		}
	}
	return 1;
}

char *underflow_repr(const int n) {
	int f[3];
	char ret[512];
	if(!underflow_repr_c(abs(n), f)) {
		int i = 0;
		const char sign = abs(n) == n ? '+' : '-';
		ret[i++] = '<';
		while (f[0]--)
			ret[i++] = '-';
		ret[i++] = '[';
		while (f[1]--)
			ret[i++] = '-';
		ret[i++] = '>';
		while (f[2]--)
			ret[i++] = sign;
		ret[i++] = '<';
		ret[i++] = ']';
		ret[i++] = '>';
		ret[i++] = 0;
		return xstrdup(ret);
	}
	return NULL;
}

void fill_lookup() {
	/*
		fill lookup tables
		shifted -255-wise for negatives
		lookup[254] = "-",
		lookup[255] = "",
		lookup[256] = "+',
	*/
	size_t last_len = 9999; /* last len to append to if good */
	for (int i = 0; i < 256; i++) {
		/* possible more compact representations: factors or underflow abuse */
		/* discard factor representation if underflow one is possible and smaller */
		char *fr = factor_repr(i), *ur = underflow_repr(i), *alt, *alt_m;
		if (ur && strlen(ur) < strlen(fr)) {
			alt = ur;
			alt_m = underflow_repr(-i);
			free(fr);
		} else {
			alt = fr;
			alt_m = factor_repr(-i);
			free(ur);
		}
		if (lookup[255+i-1] && (last_len + 1 < strlen(alt))) {
			xasprintf(&lookup[255+i], "%s%s", lookup[255+(i-1)], "+");
			xasprintf(&lookup[255-i], "%s%s", lookup[255-(i-1)], "-");
			last_len += 1;
		} else if ( strlen(alt) >= i ) {
			lookup[255+i] = raw_repr(i);
			lookup[255-i] = raw_repr(-i);
			last_len = i;
		} else {
			lookup[255+i] = alt;
			lookup[255-i] = alt_m;
			last_len = strlen(alt);
			/* only time alt and alt_m shouldn't be freed */
			continue;
		}
		free(alt);
		free(alt_m);
	}

	/* for (int i = 0; i < 255; i++)
		printf("%04d %s\n", i, lookup[255+i]); */

	/* account for when subtracting is more efficient */
	for (int i = 253; i > 15; i--) {
		const char *cur = lookup[255+i], *next = lookup[255+i+1];
		if (strlen(cur) > (strlen(next)+1)) {
			free(lookup[255+i]);
			free(lookup[255-i]);
			xasprintf(&lookup[255+i], "%s%s", next, "-");
			xasprintf(&lookup[255-i], "%s%s", lookup[255-(i+1)], "+");
		}
	}

}

int main() {
	int exit = 0;
	fill_lookup();

	exit |= puts("const char *lookup[] = {") == EOF;
	for (int i = -255; i < 256; i++)
		exit |= printf("\"%s\", /* %d */\n", lookup[255+i], i) < 0;
	exit |= puts("};") == EOF;

	return exit;
}
