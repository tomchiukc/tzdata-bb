/*
** This file is in the public domain, so clarified as of
** 1996-06-05 by Arthur David Olson.
*/

/*LINTLIBRARY*/

#include "time.h"
#include "nonstd.h"

double
difftime(time1, time0)
const time_t	time1;
const time_t	time0;
{
	return time1 - time0;
}
