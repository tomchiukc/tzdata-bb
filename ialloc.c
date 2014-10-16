/*
** This file is in the public domain, so clarified as of
** 2006-07-17 by Arthur David Olson.
*/

/*LINTLIBRARY*/

<<<<<<< HEAD
#include <stdio.h>
#include "alloc.h"

extern char *	malloc();
extern char *	realloc();
extern char *	strcpy();
extern char *	strcat();

char *	allocpy(string)
char *	string;
{
	register char *	copy;
	arg4alloc	n;

	n = (string == NULL) ? 0 : strlen(string);
	copy = malloc(++n);
	if (copy == MAL || copy == NULL)
		return NULL;
	if (string == NULL)
		*copy = '\0';
	else	(void) strcpy(copy, string);
	return copy;
}

char *	allocat(old, new)
char *	old;
char *	new;
{
	arg4alloc	n;

	n = (old == NULL) ? 0 : strlen(old);
	if (new != NULL)
		n = n + strlen(new);
	++n;
	if (old == MAL || old == NULL) {
		old = malloc(n);
		if (old == NULL)
			return NULL;
		*old = '\0';
	} else {
		old = realloc(old, n);
		if (old == NULL)
			return NULL;
	}
	if (new != NULL)
		(void) strcat(old, new);
	return old;
=======
#include "private.h"

char *
icatalloc(char *const old, const char *const new)
{
	register char *	result;
	register int	oldsize, newsize;

	newsize = (new == NULL) ? 0 : strlen(new);
	if (old == NULL)
		oldsize = 0;
	else if (newsize == 0)
		return old;
	else	oldsize = strlen(old);
	if ((result = realloc(old, oldsize + newsize + 1)) != NULL)
		if (new != NULL)
			strcpy(result + oldsize, new);
	return result;
}

char *
icpyalloc(const char *const string)
{
	return icatalloc(NULL, string);
>>>>>>> grandpa/master
}
