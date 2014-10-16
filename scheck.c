/*
** This file is in the public domain, so clarified as of
** 2006-07-17 by Arthur David Olson.
*/

/*LINTLIBRARY*/

<<<<<<< HEAD
#include "stdio.h"

#ifdef OBJECTID
static char	sccsid[] = "%W%";
#endif

#include "ctype.h"

#ifndef arg4alloc
#define arg4alloc	unsigned
#endif

#ifndef MAL
#define MAL	0
#endif

char *	scheck(string, format)
char *	string;
char *	format;
{
	register char *	fbuf;
	register char *	fp;
	register char *	tp;
	register int	c;
	register char *	result;

	if (string == NULL || format == NULL)
		return "";
	fbuf = malloc((arg4alloc) (2 * strlen(format) + 4));
	if (fbuf == MAL)
		return "";
	fp = format;
	tp = fbuf;
	while ((*tp++ = c = *fp++) != '\0') {
		if (c != '%')
			continue;
		if (*fp == '%') {
			*tp++ = *fp++;
			continue;
		}
		if (*fp == '*')
			*tp++ = *fp++;
		else	*tp++ = '*';
		while (*fp != '\0' && isascii(*fp) && isdigit(*fp))
			*tp++ = *fp++;
		if (*fp == 'l')
			*tp++ = *fp++;
		else if (*fp == '[')
			do *tp++ = *fp++;
				while (*fp != '\0' && *fp != ']');
		if ((*tp++ = *fp++) == '\0')
			break;
	}
	if (c != '\0')
		result = "";
	else if (sscanf(string, fbuf) != EOF)
		result = "";
	else	result = format;
=======
#include "private.h"

const char *
scheck(const char *const string, const char *const format)
{
	register char *		fbuf;
	register const char *	fp;
	register char *		tp;
	register int		c;
	register const char *	result;
	char			dummy;

	result = "";
	if (string == NULL || format == NULL)
		return result;
	fbuf = malloc(2 * strlen(format) + 4);
	if (fbuf == NULL)
		return result;
	fp = format;
	tp = fbuf;

	/*
	** Copy directives, suppressing each conversion that is not
	** already suppressed.  Scansets containing '%' are not
	** supported; e.g., the conversion specification "%[%]" is not
	** supported.  Also, multibyte characters containing a
	** non-leading '%' byte are not supported.
	*/
	while ((*tp++ = c = *fp++) != '\0') {
		if (c != '%')
			continue;
		if (is_digit(*fp)) {
			char const *f = fp;
			char *t = tp;
			do {
				*t++ = c = *f++;
			} while (is_digit(c));
			if (c == '$') {
				fp = f;
				tp = t;
			}
		}
		*tp++ = '*';
		if (*fp == '*')
			++fp;
		if ((*tp++ = *fp++) == '\0')
			break;
	}

	*(tp - 1) = '%';
	*tp++ = 'c';
	*tp = '\0';
	if (sscanf(string, fbuf, &dummy) != 1)
		result = format;
>>>>>>> grandpa/master
	free(fbuf);
	return result;
}
