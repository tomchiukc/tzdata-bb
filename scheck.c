#ifndef lint
#ifndef NOID
static char	elsieid[] = "%W%";
#endif /* !defined lint */
#endif /* !defined NOID */

/*LINTLIBRARY*/

#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "nonstd.h"

extern char *	imalloc P((int n));
extern void	ifree P((char * p));

char *
scheck(string, format)
const char *	string;
char *		format;
{
	register char *		fbuf;
	register const char *	fp;
	register char *		tp;
	register int		c;
	register char *		result;
	char			dummy;

	result = "";
	if (string == NULL || format == NULL)
		return result;
	fbuf = imalloc(2 * strlen(format) + 4);
	if (fbuf == NULL)
		return result;
	fp = format;
	tp = fbuf;
	while ((*tp++ = c = *fp++) != '\0') {
		if (c != '%')
			continue;
		if (*fp == '%') {
			*tp++ = *fp++;
			continue;
		}
		*tp++ = '*';
		if (*fp == '*')
			++fp;
		while (isascii(*fp) && isdigit(*fp))
			*tp++ = *fp++;
		if (*fp == 'l' || *fp == 'h')
			*tp++ = *fp++;
		else if (*fp == '[')
			do *tp++ = *fp++;
				while (*fp != '\0' && *fp != ']');
		if ((*tp++ = *fp++) == '\0')
			break;
	}
	*(tp - 1) = '%';
	*tp++ = 'c';
	*tp = '\0';
	if (sscanf(string, fbuf, &dummy) != 1)
		result = format;
	ifree(fbuf);
	return result;
}
