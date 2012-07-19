#

/*LINTLIBRARY*/

#include "stdio.h"

/*LINTLIBRARY*/

#include "private.h"
#include "tzfile.h"

/*
** Some systems only handle "%.2d"; others only handle "%02d";
** "%02.2d" makes (most) everybody happy.
** At least some versions of gcc warn about the %02.2d;
** we conditionalize below to avoid the warning.
*/
/*
** All years associated with 32-bit time_t values are exactly four digits long;
** some years associated with 64-bit time_t values are not.
** Vintage programs are coded for years that are always four digits long
** and may assume that the newline always lands in the same place.
** For years that are less than four digits, we pad the output with
** leading zeroes to get the newline in the traditional place.
** The -4 ensures that we get four characters of output even if
** we call a strftime variant that produces fewer characters for some years.
** The ISO C 1999 and POSIX 1003.1-2004 standards prohibit padding the year,
** but many implementations pad anyway; most likely the standards are buggy.
*/
#ifdef __GNUC__
#define ASCTIME_FMT	"%.3s %.3s%3d %2.2d:%2.2d:%2.2d %-4s\n"
#else /* !defined __GNUC__ */
#define ASCTIME_FMT	"%.3s %.3s%3d %02.2d:%02.2d:%02.2d %-4s\n"
#endif /* !defined __GNUC__ */
/*
** For years that are more than four digits we put extra spaces before the year
** so that code trying to overwrite the newline won't end up overwriting
** a digit within a year and truncating the year (operating on the assumption
** that no output is better than wrong output).
*/
#ifdef __GNUC__
#define ASCTIME_FMT_B	"%.3s %.3s%3d %2.2d:%2.2d:%2.2d     %s\n"
#else /* !defined __GNUC__ */
#define ASCTIME_FMT_B	"%.3s %.3s%3d %02.2d:%02.2d:%02.2d     %s\n"
#endif /* !defined __GNUC__ */

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE	1970
#endif /* !TM_YEAR_BASE */

#ifndef USG
extern char *	sprintf();
#endif /* !USG */

/*
** A la X3J11
*/

char *
asctime(timeptr)
register struct tm *	timeptr;
{
	static char	wday_name[7][3] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static char	mon_name[12][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	static char	result[26];

	(void) sprintf(result, "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n",
		wday_name[timeptr->tm_wday],
		mon_name[timeptr->tm_mon],
		timeptr->tm_mday, timeptr->tm_hour,
		timeptr->tm_min, timeptr->tm_sec,
		TM_YEAR_BASE + timeptr->tm_year);
	return result;
}
