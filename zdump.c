/*
** This file is in the public domain, so clarified as of
** 2009-05-17 by Arthur David Olson.
*/

static char	elsieid[] = "%W%";

/*
** This code has been made independent of the rest of the time
** conversion package to increase confidence in the verification it provides.
** You can use this code to help in verifying other implementations.
**
** However, include private.h when debugging, so that it overrides
** time_t consistently with the rest of the package.
*/

#ifdef time_tz
# include "private.h"
#endif

/* Enable tm_gmtoff and tm_zone on GNUish systems.  */
#define _GNU_SOURCE 1

#include "stdio.h"	/* for stdout, stderr, perror */
#include "string.h"	/* for strcpy */
#include "sys/types.h"	/* for time_t */
#include "time.h"	/* for struct tm */
#include "stdlib.h"	/* for exit, malloc, atoi */
#include "limits.h"	/* for CHAR_BIT, LLONG_MAX */
#include <errno.h>

/*
** Substitutes for pre-C99 compilers.
** Much of this section of code is stolen from private.h.
*/

#ifndef HAVE_STDINT_H
# define HAVE_STDINT_H \
    (199901 <= __STDC_VERSION__ \
     || 2 < __GLIBC__ + (1 <= __GLIBC_MINOR__)	\
     || __CYGWIN__)
#endif
#if HAVE_STDINT_H
# include "stdint.h"
#endif
#ifndef HAVE_INTTYPES_H
# define HAVE_INTTYPES_H HAVE_STDINT_H
#endif
#if HAVE_INTTYPES_H
# include <inttypes.h>
#endif

#ifndef INT_FAST32_MAX
# if INT_MAX >> 31 == 0
typedef long int_fast32_t;
# else
typedef int int_fast32_t;
# endif
#endif

#ifndef INTMAX_MAX
# if defined LLONG_MAX || defined __LONG_LONG_MAX__
typedef long long intmax_t;
#  define strtoimax strtoll
#  define PRIdMAX "lld"
#  ifdef LLONG_MAX
#   define INTMAX_MAX LLONG_MAX
#  else
#   define INTMAX_MAX __LONG_LONG_MAX__
#  endif
# else
typedef long intmax_t;
#  define strtoimax strtol
#  define PRIdMAX "ld"
#  define INTMAX_MAX LONG_MAX
# endif
#endif

/* Infer TM_ZONE on systems where this information is known, but suppress
   guessing if NO_TM_ZONE is defined.  Similarly for TM_GMTOFF.  */
#if (defined __GLIBC__ \
     || defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ \
     || (defined __APPLE__ && defined __MACH__))
# if !defined TM_GMTOFF && !defined NO_TM_GMTOFF
#  define TM_GMTOFF tm_gmtoff
# endif
# if !defined TM_ZONE && !defined NO_TM_ZONE
#  define TM_ZONE tm_zone
# endif
#endif


#ifndef ZDUMP_LO_YEAR
#define ZDUMP_LO_YEAR	(-500)
#endif /* !defined ZDUMP_LO_YEAR */

#ifndef ZDUMP_HI_YEAR
#define ZDUMP_HI_YEAR	2500
#endif /* !defined ZDUMP_HI_YEAR */

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH	1024
#endif /* !defined MAX_STRING_LENGTH */

#if __STDC_VERSION__ < 199901
# define true 1
# define false 0
# define bool int
#else
# include <stdbool.h>
#endif

#ifndef EXIT_SUCCESS
#define EXIT_SUCCESS	0
#endif /* !defined EXIT_SUCCESS */

#ifndef EXIT_FAILURE
#define EXIT_FAILURE	1
#endif /* !defined EXIT_FAILURE */

#ifndef SECSPERMIN
#define SECSPERMIN	60
#endif /* !defined SECSPERMIN */

#ifndef MINSPERHOUR
#define MINSPERHOUR	60
#endif /* !defined MINSPERHOUR */

#ifndef SECSPERHOUR
#define SECSPERHOUR	(SECSPERMIN * MINSPERHOUR)
#endif /* !defined SECSPERHOUR */

#ifndef HOURSPERDAY
#define HOURSPERDAY	24
#endif /* !defined HOURSPERDAY */

#ifndef EPOCH_YEAR
#define EPOCH_YEAR	1970
#endif /* !defined EPOCH_YEAR */

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE	1900
#endif /* !defined TM_YEAR_BASE */

#ifndef DAYSPERNYEAR
#define DAYSPERNYEAR	365
#endif /* !defined DAYSPERNYEAR */

#ifndef isleap
#define isleap(y) (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#endif /* !defined isleap */

#ifndef isleap_sum
/*
** See tzfile.h for details on isleap_sum.
*/
#define isleap_sum(a, b)	isleap((a) % 400 + (b) % 400)
#endif /* !defined isleap_sum */

#define SECSPERDAY	((int_fast32_t) SECSPERHOUR * HOURSPERDAY)
#define SECSPERNYEAR	(SECSPERDAY * DAYSPERNYEAR)
#define SECSPERLYEAR	(SECSPERNYEAR + SECSPERDAY)
#define SECSPER400YEARS	(SECSPERNYEAR * (intmax_t) (300 + 3)	\
			 + SECSPERLYEAR * (intmax_t) (100 - 3))

/*
** True if SECSPER400YEARS is known to be representable as an
** intmax_t.  It's OK that SECSPER400YEARS_FITS can in theory be false
** even if SECSPER400YEARS is representable, because when that happens
** the code merely runs a bit more slowly, and this slowness doesn't
** occur on any practical platform.
*/
enum { SECSPER400YEARS_FITS = SECSPERLYEAR <= INTMAX_MAX / 400 };

#ifndef HAVE_GETTEXT
#define HAVE_GETTEXT 0
#endif
#if HAVE_GETTEXT
#include "locale.h"	/* for setlocale */
#include "libintl.h"
#endif /* HAVE_GETTEXT */

#if 2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)
# define ATTRIBUTE_PURE __attribute__ ((__pure__))
#else
# define ATTRIBUTE_PURE /* empty */
#endif

/*
** For the benefit of GNU folk...
** '_(MSGID)' uses the current locale's message library string for MSGID.
** The default is to use gettext if available, and use MSGID otherwise.
*/

#ifndef _
#if HAVE_GETTEXT
#define _(msgid) gettext(msgid)
#else /* !HAVE_GETTEXT */
#define _(msgid) msgid
#endif /* !HAVE_GETTEXT */
#endif /* !defined _ */

#if !defined TZ_DOMAIN && defined HAVE_GETTEXT
# define TZ_DOMAIN "tz"
#endif

extern char **	environ;
extern int	getopt(int argc, char * const argv[],
			const char * options);
extern char *	optarg;
extern int	optind;
extern char *	tzname[2];

/* The minimum and maximum finite time values.  */
static time_t const absolute_min_time =
  ((time_t) -1 < 0
   ? (time_t) -1 << (CHAR_BIT * sizeof (time_t) - 1)
   : 0);
static time_t const absolute_max_time =
  ((time_t) -1 < 0
   ? - (~ 0 < 0) - ((time_t) -1 << (CHAR_BIT * sizeof (time_t) - 1))
   : -1);
static int	longest;
static char *	progname;
static bool	warned;
static bool	errout;

static char const *abbr(struct tm const *);
static intmax_t	delta(struct tm *, struct tm *) ATTRIBUTE_PURE;
static void dumptime(struct tm const *);
static time_t hunt(char *, time_t, time_t);
static void show(char *, time_t, bool);
static const char *tformat(void);
static time_t yeartot(intmax_t) ATTRIBUTE_PURE;

/* Is A an alphabetic character in the C locale?  */
static bool
is_alpha(char a)
{
	switch (a) {
	  default:
		return false;
	  case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
	  case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
	  case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
	  case 'V': case 'W': case 'X': case 'Y': case 'Z':
	  case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
	  case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
	  case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
	  case 'v': case 'w': case 'x': case 'y': case 'z':
		return true;
	}
}

/* Return A + B, exiting if the result would overflow.  */
static size_t
sumsize(size_t a, size_t b)
{
  size_t sum = a + b;
  if (sum < a) {
    fprintf(stderr, "%s: size overflow\n", progname);
    exit(EXIT_FAILURE);
  }
  return sum;
}

/* Set the global time zone to VAL, exiting on memory allocation failure.  */
static void
settimezone(char const *val)
{
  static char **fakeenv;
  char **env = fakeenv;
  char *oldstorage = env ? env[0] : 0;
  char *env0;
  if (! env) {
    char **e = environ;
    int to;

    while (*e++)
      continue;
    env = malloc(sumsize(sizeof *environ,
			 (e - environ) * sizeof *environ));
    if (! env) {
      perror(progname);
      exit(EXIT_FAILURE);
    }
    to = 1;
    for (e = environ; (env[to] = *e); e++)
      to += strncmp(*e, "TZ=", 3) != 0;
  }
  env0 = malloc(sumsize(sizeof "TZ=", strlen(val)));
  if (! env0) {
    perror(progname);
    exit(EXIT_FAILURE);
  }
  env[0] = strcat(strcpy(env0, "TZ="), val);
  environ = fakeenv = env;
  free(oldstorage);
}

#ifndef TYPECHECK
#define my_localtime	localtime
#else /* !defined TYPECHECK */
static struct tm *
my_localtime(time_t *tp)
{
	register struct tm *	tmp;

	tmp = localtime(tp);
	if (tp != NULL && tmp != NULL) {
		struct tm	tm;
		register time_t	t;

		tm = *tmp;
		t = mktime(&tm);
		if (t != *tp) {
			fflush(stdout);
			fprintf(stderr, "\n%s: ", progname);
			fprintf(stderr, tformat(), *tp);
			fprintf(stderr, " ->");
			fprintf(stderr, " year=%d", tmp->tm_year);
			fprintf(stderr, " mon=%d", tmp->tm_mon);
			fprintf(stderr, " mday=%d", tmp->tm_mday);
			fprintf(stderr, " hour=%d", tmp->tm_hour);
			fprintf(stderr, " min=%d", tmp->tm_min);
			fprintf(stderr, " sec=%d", tmp->tm_sec);
			fprintf(stderr, " isdst=%d", tmp->tm_isdst);
			fprintf(stderr, " -> ");
			fprintf(stderr, tformat(), t);
			fprintf(stderr, "\n");
			errout = true;
		}
	}
	return tmp;
}
#endif /* !defined TYPECHECK */

static void
abbrok(const char *const abbrp, const char *const zone)
{
	register const char *	cp;
	register const char *	wp;

	if (warned)
		return;
	cp = abbrp;
	wp = NULL;
	while (is_alpha(*cp))
		++cp;
	if (cp - abbrp == 0)
		wp = _("lacks alphabetic at start");
	else if (cp - abbrp < 3)
		wp = _("has fewer than 3 alphabetics");
	else if (cp - abbrp > 6)
		wp = _("has more than 6 alphabetics");
	if (wp == NULL && (*cp == '+' || *cp == '-')) {
		++cp;
		if ('0' <= *cp && *cp <= '9')
			if (*cp++ == '1' && '0' <= *cp && *cp <= '4')
				cp++;
		if (*cp != '\0')
			wp = _("differs from POSIX standard");
	}
	if (wp == NULL)
		return;
	fflush(stdout);
	fprintf(stderr,
		_("%s: warning: zone \"%s\" abbreviation \"%s\" %s\n"),
		progname, zone, abbrp, wp);
	warned = errout = true;
}

/* Save into *BUF (of size *BUFALLOC) the time zone abbreviation of TMP.
   Exit on memory allocation failure.  */
static void
saveabbr(char **buf, size_t *bufalloc, struct tm const *tmp)
{
  char const *ab = abbr(tmp);
  size_t ablen = strlen(ab);
  if (*bufalloc <= ablen) {
    free(*buf);

    /* Make the new buffer at least twice as long as the old,
       to avoid O(N**2) behavior on repeated calls.  */
    *bufalloc = sumsize(*bufalloc, ablen + 1);

    *buf = malloc(*bufalloc);
    if (! *buf) {
      perror(progname);
      exit(EXIT_FAILURE);
    }
  }
  strcpy(*buf, ab);
}

static void
close_file(FILE *stream)
{
  char const *e = (ferror(stream) ? _("I/O error")
		   : fclose(stream) != 0 ? strerror(errno) : NULL);
  if (e) {
    fprintf(stderr, "%s: %s\n", progname, e);
    exit(EXIT_FAILURE);
  }
}

static void
usage(FILE * const stream, const int status)
{
	fprintf(stream,
_("%s: usage: %s [--version] [--help] [-{vV}] [-{ct} [lo,]hi] zonename ...\n"
  "\n"
  "Report bugs to %s.\n"),
		       progname, progname, REPORT_BUGS_TO);
	if (status == EXIT_SUCCESS)
	  close_file(stream);
	exit(status);
}

int
main(int argc, char *argv[])
{
	/* These are static so that they're initially zero.  */
	static char *		abbrev;
	static size_t		abbrevsize;
	static struct tm	newtm;

	register int		i;
	register bool		vflag;
	register bool		Vflag;
	register char *		cutarg;
	register char *		cuttimes;
	register time_t		cutlotime;
	register time_t		cuthitime;
	time_t			now;
	time_t			t;
	time_t			newt;
	struct tm		tm;
	register struct tm *	tmp;
	register struct tm *	newtmp;

	cutlotime = absolute_min_time;
	cuthitime = absolute_max_time;
#if HAVE_GETTEXT
	setlocale(LC_ALL, "");
#ifdef TZ_DOMAINDIR
	bindtextdomain(TZ_DOMAIN, TZ_DOMAINDIR);
#endif /* defined TEXTDOMAINDIR */
	textdomain(TZ_DOMAIN);
#endif /* HAVE_GETTEXT */
	progname = argv[0];
	for (i = 1; i < argc; ++i)
		if (strcmp(argv[i], "--version") == 0) {
			printf("zdump %s%s\n", PKGVERSION, TZVERSION);
			return EXIT_SUCCESS;
		} else if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, EXIT_SUCCESS);
		}
	vflag = Vflag = false;
	cutarg = cuttimes = NULL;
	for (;;)
	  switch (getopt(argc, argv, "c:t:vV")) {
	  case 'c': cutarg = optarg; break;
	  case 't': cuttimes = optarg; break;
	  case 'v': vflag = true; break;
	  case 'V': Vflag = true; break;
	  case -1:
	    if (! (optind == argc - 1 && strcmp(argv[optind], "=") == 0))
	      goto arg_processing_done;
	    /* Fall through.  */
	  default:
	    usage(stderr, EXIT_FAILURE);
	  }
 arg_processing_done:;

	if (vflag | Vflag) {
		intmax_t	lo;
		intmax_t	hi;
		char *loend, *hiend;
		register intmax_t cutloyear = ZDUMP_LO_YEAR;
		register intmax_t cuthiyear = ZDUMP_HI_YEAR;
		if (cutarg != NULL) {
			lo = strtoimax(cutarg, &loend, 10);
			if (cutarg != loend && !*loend) {
				hi = lo;
				cuthiyear = hi;
			} else if (cutarg != loend && *loend == ','
				   && (hi = strtoimax(loend + 1, &hiend, 10),
				       loend + 1 != hiend && !*hiend)) {
				cutloyear = lo;
				cuthiyear = hi;
			} else {
				fprintf(stderr, _("%s: wild -c argument %s\n"),
					progname, cutarg);
				return EXIT_FAILURE;
			}
		}
		if (cutarg != NULL || cuttimes == NULL) {
			cutlotime = yeartot(cutloyear);
			cuthitime = yeartot(cuthiyear);
		}
		if (cuttimes != NULL) {
			lo = strtoimax(cuttimes, &loend, 10);
			if (cuttimes != loend && !*loend) {
				hi = lo;
				if (hi < cuthitime) {
					if (hi < absolute_min_time)
						hi = absolute_min_time;
					cuthitime = hi;
				}
			} else if (cuttimes != loend && *loend == ','
				   && (hi = strtoimax(loend + 1, &hiend, 10),
				       loend + 1 != hiend && !*hiend)) {
				if (cutlotime < lo) {
					if (absolute_max_time < lo)
						lo = absolute_max_time;
					cutlotime = lo;
				}
				if (hi < cuthitime) {
					if (hi < absolute_min_time)
						hi = absolute_min_time;
					cuthitime = hi;
				}
			} else {
				fprintf(stderr,
					_("%s: wild -t argument %s\n"),
					progname, cuttimes);
				return EXIT_FAILURE;
			}
		}
	}
	now = time(NULL);
	longest = 0;
	for (i = optind; i < argc; i++) {
	  size_t arglen = strlen(argv[i]);
	  if (longest < arglen)
	    longest = arglen < INT_MAX ? arglen : INT_MAX;
	}

	for (i = optind; i < argc; ++i) {
		settimezone(argv[i]);
		if (! (vflag | Vflag)) {
			show(argv[i], now, false);
			continue;
		}
		warned = false;
		t = absolute_min_time;
		if (!Vflag) {
			show(argv[i], t, true);
			t += SECSPERDAY;
			show(argv[i], t, true);
		}
		if (t < cutlotime)
			t = cutlotime;
		tmp = my_localtime(&t);
		if (tmp != NULL) {
			tm = *tmp;
			saveabbr(&abbrev, &abbrevsize, &tm);
		}
		for ( ; ; ) {
			newt = (t < absolute_max_time - SECSPERDAY / 2
				? t + SECSPERDAY / 2
				: absolute_max_time);
			if (cuthitime <= newt)
				break;
			newtmp = localtime(&newt);
			if (newtmp != NULL)
				newtm = *newtmp;
			if ((tmp == NULL || newtmp == NULL) ? (tmp != newtmp) :
				(delta(&newtm, &tm) != (newt - t) ||
				newtm.tm_isdst != tm.tm_isdst ||
				strcmp(abbr(&newtm), abbrev) != 0)) {
					newt = hunt(argv[i], t, newt);
					newtmp = localtime(&newt);
					if (newtmp != NULL) {
						newtm = *newtmp;
						saveabbr(&abbrev, &abbrevsize,
							 &newtm);
					}
			}
			t = newt;
			tm = newtm;
			tmp = newtmp;
		}
		if (!Vflag) {
			t = absolute_max_time;
			t -= SECSPERDAY;
			show(argv[i], t, true);
			t += SECSPERDAY;
			show(argv[i], t, true);
		}
	}
	close_file(stdout);
	if (errout && (ferror(stderr) || fclose(stderr) != 0))
	  return EXIT_FAILURE;
	return EXIT_SUCCESS;
}

static time_t
yeartot(const intmax_t y)
{
	register intmax_t	myy, seconds, years;
	register time_t		t;

	myy = EPOCH_YEAR;
	t = 0;
	while (myy < y) {
		if (SECSPER400YEARS_FITS && 400 <= y - myy) {
			intmax_t diff400 = (y - myy) / 400;
			if (INTMAX_MAX / SECSPER400YEARS < diff400)
				return absolute_max_time;
			seconds = diff400 * SECSPER400YEARS;
			years = diff400 * 400;
                } else {
			seconds = isleap(myy) ? SECSPERLYEAR : SECSPERNYEAR;
			years = 1;
		}
		myy += years;
		if (t > absolute_max_time - seconds)
			return absolute_max_time;
		t += seconds;
	}
	while (y < myy) {
		if (SECSPER400YEARS_FITS && y + 400 <= myy && myy < 0) {
			intmax_t diff400 = (myy - y) / 400;
			if (INTMAX_MAX / SECSPER400YEARS < diff400)
				return absolute_min_time;
			seconds = diff400 * SECSPER400YEARS;
			years = diff400 * 400;
		} else {
			seconds = isleap(myy - 1) ? SECSPERLYEAR : SECSPERNYEAR;
			years = 1;
		}
		myy -= years;
		if (t < absolute_min_time + seconds)
			return absolute_min_time;
		t -= seconds;
	}
	return t;
}

static time_t
hunt(char *name, time_t lot, time_t hit)
{
	static char *		loab;
	static size_t		loabsize;
	time_t			t;
	struct tm		lotm;
	register struct tm *	lotmp;
	struct tm		tm;
	register struct tm *	tmp;

	lotmp = my_localtime(&lot);
	if (lotmp != NULL) {
		lotm = *lotmp;
		saveabbr(&loab, &loabsize, &lotm);
	}
	for ( ; ; ) {
		time_t diff = hit - lot;
		if (diff < 2)
			break;
		t = lot;
		t += diff / 2;
		if (t <= lot)
			++t;
		else if (t >= hit)
			--t;
		tmp = my_localtime(&t);
		if (tmp != NULL)
			tm = *tmp;
		if ((lotmp == NULL || tmp == NULL) ? (lotmp == tmp) :
			(delta(&tm, &lotm) == (t - lot) &&
			tm.tm_isdst == lotm.tm_isdst &&
			strcmp(abbr(&tm), loab) == 0)) {
				lot = t;
				lotm = tm;
				lotmp = tmp;
		} else	hit = t;
	}
	show(name, lot, true);
	show(name, hit, true);
	return hit;
}

/*
** Thanks to Paul Eggert for logic used in delta.
*/

static intmax_t
delta(struct tm * newp, struct tm *oldp)
{
	register intmax_t	result;
	register int		tmy;

	if (newp->tm_year < oldp->tm_year)
		return -delta(oldp, newp);
	result = 0;
	for (tmy = oldp->tm_year; tmy < newp->tm_year; ++tmy)
		result += DAYSPERNYEAR + isleap_sum(tmy, TM_YEAR_BASE);
	result += newp->tm_yday - oldp->tm_yday;
	result *= HOURSPERDAY;
	result += newp->tm_hour - oldp->tm_hour;
	result *= MINSPERHOUR;
	result += newp->tm_min - oldp->tm_min;
	result *= SECSPERMIN;
	result += newp->tm_sec - oldp->tm_sec;
	return result;
}

static void
show(char *zone, time_t t, bool v)
{
	register struct tm *	tmp;

	printf("%-*s  ", longest, zone);
	if (v) {
		tmp = gmtime(&t);
		if (tmp == NULL) {
			printf(tformat(), t);
		} else {
			dumptime(tmp);
			printf(" UT");
		}
		printf(" = ");
	}
	tmp = my_localtime(&t);
	dumptime(tmp);
	if (tmp != NULL) {
		if (*abbr(tmp) != '\0')
			printf(" %s", abbr(tmp));
		if (v) {
			printf(" isdst=%d", tmp->tm_isdst);
#ifdef TM_GMTOFF
			printf(" gmtoff=%ld", tmp->TM_GMTOFF);
#endif /* defined TM_GMTOFF */
		}
	}
	printf("\n");
	if (tmp != NULL && *abbr(tmp) != '\0')
		abbrok(abbr(tmp), zone);
}

static char const *
abbr(struct tm const *tmp)
{
#ifdef TM_ZONE
	return tmp->TM_ZONE;
#else
	return ((0 <= tmp->tm_isdst && tmp->tm_isdst <= 1
		 && tzname[tmp->tm_isdst])
		? tzname[tmp->tm_isdst]
		: "");
#endif
}

/*
** The code below can fail on certain theoretical systems;
** it works on all known real-world systems as of 2004-12-30.
*/

static const char *
tformat(void)
{
	if (0 > (time_t) -1) {		/* signed */
		if (sizeof (time_t) == sizeof (intmax_t))
			return "%"PRIdMAX;
		if (sizeof (time_t) > sizeof (long))
			return "%lld";
		if (sizeof (time_t) > sizeof (int))
			return "%ld";
		return "%d";
	}
#ifdef PRIuMAX
	if (sizeof (time_t) == sizeof (uintmax_t))
		return "%"PRIuMAX;
#endif
	if (sizeof (time_t) > sizeof (unsigned long))
		return "%llu";
	if (sizeof (time_t) > sizeof (unsigned int))
		return "%lu";
	return "%u";
}

static void
dumptime(register const struct tm *timeptr)
{
	static const char	wday_name[][3] = {
		"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
	};
	static const char	mon_name[][3] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};
	register const char *	wn;
	register const char *	mn;
	register int		lead;
	register int		trail;

	if (timeptr == NULL) {
		printf("NULL");
		return;
	}
	/*
	** The packaged versions of localtime and gmtime never put out-of-range
	** values in tm_wday or tm_mon, but since this code might be compiled
	** with other (perhaps experimental) versions, paranoia is in order.
	*/
	if (timeptr->tm_wday < 0 || timeptr->tm_wday >=
		(int) (sizeof wday_name / sizeof wday_name[0]))
			wn = "???";
	else		wn = wday_name[timeptr->tm_wday];
	if (timeptr->tm_mon < 0 || timeptr->tm_mon >=
		(int) (sizeof mon_name / sizeof mon_name[0]))
			mn = "???";
	else		mn = mon_name[timeptr->tm_mon];
	printf("%.3s %.3s%3d %.2d:%.2d:%.2d ",
		wn, mn,
		timeptr->tm_mday, timeptr->tm_hour,
		timeptr->tm_min, timeptr->tm_sec);
#define DIVISOR	10
	trail = timeptr->tm_year % DIVISOR + TM_YEAR_BASE % DIVISOR;
	lead = timeptr->tm_year / DIVISOR + TM_YEAR_BASE / DIVISOR +
		trail / DIVISOR;
	trail %= DIVISOR;
	if (trail < 0 && lead > 0) {
		trail += DIVISOR;
		--lead;
	} else if (lead < 0 && trail > 0) {
		trail -= DIVISOR;
		++lead;
	}
	if (lead == 0)
		printf("%d", trail);
	else	printf("%d%d", lead, ((trail < 0) ? -trail : trail));
}
