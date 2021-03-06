/*
 * Copyright (c) 1985, 1987, 1988 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

<<<<<<< HEAD
#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1985, 1987, 1988 The Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)date.c	4.23 (Berkeley) 9/20/88";
#endif /* not lint */

/*
 * Date - print and set date
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/file.h>
#include <errno.h>
#include <syslog.h>
#include <utmp.h>
#include <tzfile.h>
#include <stdio.h>
#include <ctype.h>
#include <strings.h>

#define	ATOI2(ar)	(ar[0] - '0') * 10 + (ar[1] - '0'); ar += 2;

static struct timeval	tv;
static int	retval;

static int	dmsize[] =
	{ -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
=======
#include "private.h"
#if HAVE_ADJTIME || HAVE_SETTIMEOFDAY
#include "sys/time.h"	/* for struct timeval, struct timezone */
#endif /* HAVE_ADJTIME || HAVE_SETTIMEOFDAY */
#include "locale.h"
#include "utmp.h"	/* for OLD_TIME (or its absence) */
#if HAVE_UTMPX_H
#include "utmpx.h"
#endif

#ifndef OTIME_MSG
#define OTIME_MSG "old time"
#endif
#ifndef NTIME_MSG
#define NTIME_MSG "new time"
#endif
#if !defined WTMPX_FILE && defined _PATH_WTMPX
# define WTMPX_FILE _PATH_WTMPX
#endif

/*
** The two things date knows about time are. . .
*/
>>>>>>> grandpa/master

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE	1900
#endif /* !defined TM_YEAR_BASE */

#ifndef SECSPERMIN
#define SECSPERMIN	60
#endif /* !defined SECSPERMIN */

extern char **		environ;
extern char *		optarg;
extern int		optind;
extern char *		tzname[2];

static int		retval = EXIT_SUCCESS;

static void		checkfinal(char const *, bool, time_t, time_t);
static time_t		convert(const char *, bool, time_t);
static void		display(const char *, time_t);
static void		dogmt(void);
static void		errensure(void);
static void		iffy(time_t, time_t, const char *, const char *);
static const char *	nondigit(const char *);
static void		oops(const char *);
static void		reset(time_t, bool);
static void		timeout(FILE *, const char *, const struct tm *);
static void		usage(void);
static void		wildinput(const char *, const char *,
				const char *);

int
main(const int argc, char *argv[])
{
<<<<<<< HEAD
	extern int optind;
	extern char *optarg;
	struct timezone tz;
	char *ap, *tzn;
	int ch, uflag, nflag;
	char *username, *getlogin();
	time_t time();

	nflag = uflag = 0;
	tz.tz_dsttime = tz.tz_minuteswest = 0;
	while ((ch = getopt(argc, argv, "d:nut:")) != EOF)
		switch((char)ch) {
		case 'd':		/* daylight savings time */
			tz.tz_dsttime = atoi(optarg) ? 1 : 0;
			break;
		case 'n':		/* don't set network */
			nflag = 1;
			break;
		case 'u':		/* do it in GMT */
			uflag = 1;
			break;
		case 't':		/* minutes west of GMT */
					/* error check; we can't allow "PST" */
			if (isdigit(*optarg)) {
				tz.tz_minuteswest = atoi(optarg);
				break;
			}
			/*FALLTHROUGH*/
		default:
			usage();
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if (argc > 1) {
		usage();
		exit(1);
	}

	if ((tz.tz_minuteswest || tz.tz_dsttime) &&
	    settimeofday((struct timeval *)NULL, &tz)) {
		perror("settimeofday");
		retval = 1;
		goto display;
	}
}

#ifdef OLD_TIME

/*
** We assume we're on a System-V-based system,
** should use stime,
** should write System-V-format utmp entries,
** and don't have network notification to worry about.
*/

#include "fcntl.h"	/* for O_WRONLY, O_APPEND */

/*ARGSUSED*/
static void
reset(const time_t newt, const int nflag)
{
	register int		fid;
	time_t			oldt;
	static struct {
		struct utmp	before;
		struct utmp	after;
	} s;
#if HAVE_UTMPX_H
	static struct {
		struct utmpx	before;
		struct utmpx	after;
	} sx;
#endif

	/*
	** Wouldn't it be great if stime returned the old time?
	*/
	(void) time(&oldt);
	if (stime(&newt) != 0)
		oops("stime");
	s.before.ut_type = OLD_TIME;
	s.before.ut_time = oldt;
	(void) strcpy(s.before.ut_line, OTIME_MSG);
	s.after.ut_type = NEW_TIME;
	s.after.ut_time = newt;
	(void) strcpy(s.after.ut_line, NTIME_MSG);
	fid = open(WTMP_FILE, O_WRONLY | O_APPEND);
	if (fid < 0)
		oops(_("log file open"));
	if (write(fid, (char *) &s, sizeof s) != sizeof s)
		oops(_("log file write"));
	if (close(fid) != 0)
		oops(_("log file close"));
#if !HAVE_UTMPX_H
	pututline(&s.before);
	pututline(&s.after);
#endif /* !HAVE_UTMPX_H */
#if HAVE_UTMPX_H
	sx.before.ut_type = OLD_TIME;
	sx.before.ut_tv.tv_sec = oldt;
	(void) strcpy(sx.before.ut_line, OTIME_MSG);
	sx.after.ut_type = NEW_TIME;
	sx.after.ut_tv.tv_sec = newt;
	(void) strcpy(sx.after.ut_line, NTIME_MSG);
#if !SUPPRESS_WTMPX_FILE_UPDATE
	/* In Solaris 2.5 (and presumably other systems),
	   'date' does not update /var/adm/wtmpx.
	   This must be a bug.  If you'd like to reproduce the bug,
	   define SUPPRESS_WTMPX_FILE_UPDATE to be nonzero.  */
	fid = open(WTMPX_FILE, O_WRONLY | O_APPEND);
	if (fid < 0)
		oops(_("log file open"));
	if (write(fid, (char *) &sx, sizeof sx) != sizeof sx)
		oops(_("log file write"));
	if (close(fid) != 0)
		oops(_("log file close"));
#endif /* !SUPPRESS_WTMPX_FILE_UPDATE */
	pututxline(&sx.before);
	pututxline(&sx.after);
#endif /* HAVE_UTMPX_H */
}

#endif /* defined OLD_TIME */
#ifndef OLD_TIME

/*
** We assume we're on a BSD-based system,
** should use settimeofday,
** should write BSD-format utmp entries (using logwtmp),
** and may get to worry about network notification.
** The "time name" changes between 4.3-tahoe and 4.4;
** we include sys/param.h to determine which we should use.
*/

#ifndef TIME_NAME
#include "sys/param.h"
#ifdef BSD4_4
#define TIME_NAME	"date"
#endif /* defined BSD4_4 */
#ifndef BSD4_4
#define TIME_NAME	""
#endif /* !defined BSD4_4 */
#endif /* !defined TIME_NAME */

#include "syslog.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#define TSPTYPES
#include "protocols/timed.h"

=======
	register const char *	format;
	register const char *	value;
	register const char *	cp;
	register int		ch;
	register bool		dousg;
	register bool		aflag = false;
	register bool		dflag = false;
	register bool		nflag = false;
	register bool		tflag = false;
	register bool		rflag = false;
	register int		minuteswest;
	register int		dsttime;
	register double		adjust;
	time_t			now;
	time_t			t;
	intmax_t		secs;
	char *			endarg;

	INITIALIZE(dousg);
#ifdef LC_ALL
	setlocale(LC_ALL, "");
#endif /* defined(LC_ALL) */
#if HAVE_GETTEXT
#ifdef TZ_DOMAINDIR
	bindtextdomain(TZ_DOMAIN, TZ_DOMAINDIR);
#endif /* defined(TEXTDOMAINDIR) */
	textdomain(TZ_DOMAIN);
#endif /* HAVE_GETTEXT */
	t = now = time(NULL);
	format = value = NULL;
	while ((ch = getopt(argc, argv, "ucr:nd:t:a:")) != EOF && ch != -1) {
		switch (ch) {
		default:
			usage();
		case 'u':		/* do it in UT */
		case 'c':
			dogmt();
			break;
		case 'r':		/* seconds since 1970 */
			if (rflag) {
				fprintf(stderr,
					_("date: error: multiple -r's used"));
				usage();
			}
			rflag = true;
			errno = 0;
			secs = strtoimax (optarg, &endarg, 0);
			if (*endarg || optarg == endarg)
				errno = EINVAL;
			else if (! (time_t_min <= secs && secs <= time_t_max))
				errno = ERANGE;
			if (errno) {
				perror(optarg);
				errensure();
				exit(retval);
			}
			t = secs;
			break;
		case 'n':		/* don't set network */
			nflag = true;
			break;
		case 'd':		/* daylight saving time */
			if (dflag) {
				fprintf(stderr,
					_("date: error: multiple -d's used"));
				usage();
			}
			dflag = true;
			cp = optarg;
			dsttime = atoi(cp);
			if (*cp == '\0' || *nondigit(cp) != '\0')
				wildinput(_("-t value"), optarg,
					_("must be a non-negative number"));
			break;
		case 't':		/* minutes west of UTC */
			if (tflag) {
				fprintf(stderr,
					_("date: error: multiple -t's used"));
				usage();
			}
			tflag = true;
			cp = optarg;
			minuteswest = atoi(cp);
			if (*cp == '+' || *cp == '-')
				++cp;
			if (*cp == '\0' || *nondigit(cp) != '\0')
				wildinput(_("-d value"), optarg,
					_("must be a number"));
			break;
		case 'a':		/* adjustment */
			if (aflag) {
				fprintf(stderr,
					_("date: error: multiple -a's used"));
				usage();
			}
			aflag = true;
			cp = optarg;
			adjust = atof(cp);
			if (*cp == '+' || *cp == '-')
				++cp;
			if (*cp == '\0' || strcmp(cp, ".") == 0)
				wildinput(_("-a value"), optarg,
					_("must be a number"));
			cp = nondigit(cp);
			if (*cp == '.')
				++cp;
			if (*nondigit(cp) != '\0')
				wildinput(_("-a value"), optarg,
					_("must be a number"));
			break;
		}
	}
	while (optind < argc) {
		cp = argv[optind++];
		if (*cp == '+')
			if (format == NULL)
				format = cp + 1;
			else {
				fprintf(stderr,
_("date: error: multiple formats in command line\n"));
				usage();
			}
		else	if (value == NULL && !rflag)
				value = cp;
			else {
				fprintf(stderr,
_("date: error: multiple values in command line\n"));
				usage();
			}
	}
	if (value != NULL) {
		/*
		** This order ensures that "reasonable" twelve-digit inputs
		** (such as 120203042006) won't be misinterpreted
		** even if time_t's range all the way back to the thirteenth
		** century.  Do not change the order.
		*/
		t = convert(value, (dousg = true), now);
		if (t == -1)
			t = convert(value, (dousg = false), now);
		if (t == -1) {
			/*
			** Out of range values,
			** or time that falls in a DST transition hole?
			*/
			if ((cp = strchr(value, '.')) != NULL) {
				/*
				** Ensure that the failure of
				**	TZ=America/New_York date 8712312359.60
				** doesn't get misdiagnosed.  (It was
				**	TZ=America/New_York date 8712311859.60
				** when the leap second was inserted.)
				** The normal check won't work since
				** the given time is valid in UTC.
				*/
				if (atoi(cp + 1) >= SECSPERMIN)
					wildinput(_("time"), value,
					    _("out of range seconds given"));
			}
			dogmt();
			t = convert(value, false, now);
			if (t == -1)
				t = convert(value, true, now);
			wildinput(_("time"), value,
				(t == -1) ?
				_("out of range value given") :
				_("time skipped when clock springs forward"));
		}
	}
	/*
	** Entire command line has now been checked.
	*/
	if (aflag) {
#if HAVE_ADJTIME
		struct timeval	tv;

		tv.tv_sec = (int) adjust;
		tv.tv_usec = (int) ((adjust - tv.tv_sec) * 1000000L);
		if (adjtime(&tv, NULL) != 0)
			oops("adjtime");
#endif /* HAVE_ADJTIME */
#if !HAVE_ADJTIME
		reset(now + adjust, nflag);
#endif /* !HAVE_ADJTIME */
		/*
		** Sun silently ignores everything else; we follow suit.
		*/
		exit(retval);
	}
	if (dflag || tflag) {
#if HAVE_SETTIMEOFDAY == 2
		struct timezone	tz;

		if (!dflag || !tflag)
			if (gettimeofday(NULL, &tz) != 0)
				oops("gettimeofday");
		if (dflag)
			tz.tz_dsttime = dsttime;
		if (tflag)
			tz.tz_minuteswest = minuteswest;
		if (settimeofday(NULL, &tz) != 0)
			oops("settimeofday");
#endif /* HAVE_SETTIMEOFDAY == 2 */
#if HAVE_SETTIMEOFDAY != 2
		(void) dsttime;
		(void) minuteswest;
		fprintf(stderr,
_("date: warning: kernel doesn't keep -d/-t information, option ignored\n"));
#endif /* HAVE_SETTIMEOFDAY != 2 */
	}

	if (value) {
		reset(t, nflag);
		checkfinal(value, dousg, t, now);
		t = time(NULL);
	}

	display(format, t);
	return retval;
}

static void
dogmt(void)
{
	static char **	fakeenv;

	if (fakeenv == NULL) {
		register int	from;
		register int	to;
		register int	n;
		static char	tzegmt0[] = "TZ=GMT0";

		for (n = 0;  environ[n] != NULL;  ++n)
			continue;
		fakeenv = malloc((n + 2) * sizeof *fakeenv);
		if (fakeenv == NULL) {
			perror(_("Memory exhausted"));
			errensure();
			exit(retval);
		}
		to = 0;
		fakeenv[to++] = tzegmt0;
		for (from = 1; environ[from] != NULL; ++from)
			if (strncmp(environ[from], "TZ=", 3) != 0)
				fakeenv[to++] = environ[from];
		fakeenv[to] = NULL;
		environ = fakeenv;
	}
}

>>>>>>> grandpa/master
#ifdef OLD_TIME

/*
** We assume we're on a System-V-based system,
** should use stime,
** should write System-V-format utmp entries,
** and don't have network notification to worry about.
*/

#include "fcntl.h"	/* for O_WRONLY, O_APPEND */

/*ARGSUSED*/
static void
<<<<<<< HEAD
reset(const time_t newt, const int nflag)
=======
reset(time_t newt, bool nflag)
>>>>>>> grandpa/master
{
	register int		fid;
	time_t			oldt;
	static struct {
		struct utmp	before;
		struct utmp	after;
	} s;
#if HAVE_UTMPX_H
	static struct {
		struct utmpx	before;
		struct utmpx	after;
	} sx;
#endif

	/*
	** Wouldn't it be great if stime returned the old time?
	*/
<<<<<<< HEAD
	(void) time(&oldt);
=======
	oldt = time(NULL);
>>>>>>> grandpa/master
	if (stime(&newt) != 0)
		oops("stime");
	s.before.ut_type = OLD_TIME;
	s.before.ut_time = oldt;
<<<<<<< HEAD
	(void) strcpy(s.before.ut_line, OTIME_MSG);
	s.after.ut_type = NEW_TIME;
	s.after.ut_time = newt;
	(void) strcpy(s.after.ut_line, NTIME_MSG);
=======
	strcpy(s.before.ut_line, OTIME_MSG);
	s.after.ut_type = NEW_TIME;
	s.after.ut_time = newt;
	strcpy(s.after.ut_line, NTIME_MSG);
>>>>>>> grandpa/master
	fid = open(WTMP_FILE, O_WRONLY | O_APPEND);
	if (fid < 0)
		oops(_("log file open"));
	if (write(fid, (char *) &s, sizeof s) != sizeof s)
		oops(_("log file write"));
	if (close(fid) != 0)
		oops(_("log file close"));
#if !HAVE_UTMPX_H
	pututline(&s.before);
	pututline(&s.after);
#endif /* !HAVE_UTMPX_H */
#if HAVE_UTMPX_H
	sx.before.ut_type = OLD_TIME;
	sx.before.ut_tv.tv_sec = oldt;
<<<<<<< HEAD
	(void) strcpy(sx.before.ut_line, OTIME_MSG);
	sx.after.ut_type = NEW_TIME;
	sx.after.ut_tv.tv_sec = newt;
	(void) strcpy(sx.after.ut_line, NTIME_MSG);
#if !SUPPRESS_WTMPX_FILE_UPDATE
=======
	strcpy(sx.before.ut_line, OTIME_MSG);
	sx.after.ut_type = NEW_TIME;
	sx.after.ut_tv.tv_sec = newt;
	strcpy(sx.after.ut_line, NTIME_MSG);
#if defined WTMPX_FILE && !SUPPRESS_WTMPX_FILE_UPDATE
>>>>>>> grandpa/master
	/* In Solaris 2.5 (and presumably other systems),
	   'date' does not update /var/adm/wtmpx.
	   This must be a bug.  If you'd like to reproduce the bug,
	   define SUPPRESS_WTMPX_FILE_UPDATE to be nonzero.  */
	fid = open(WTMPX_FILE, O_WRONLY | O_APPEND);
	if (fid < 0)
		oops(_("log file open"));
	if (write(fid, (char *) &sx, sizeof sx) != sizeof sx)
		oops(_("log file write"));
	if (close(fid) != 0)
		oops(_("log file close"));
<<<<<<< HEAD
#endif /* !SUPPRESS_WTMPX_FILE_UPDATE */
=======
# endif
>>>>>>> grandpa/master
	pututxline(&sx.before);
	pututxline(&sx.after);
#endif /* HAVE_UTMPX_H */
}

#endif /* defined OLD_TIME */
#ifndef OLD_TIME

/*
** We assume we're on a BSD-based system,
** should use settimeofday,
** should write BSD-format utmp entries (using logwtmp),
** and may get to worry about network notification.
** The "time name" changes between 4.3-tahoe and 4.4;
** we include sys/param.h to determine which we should use.
*/

#ifndef TIME_NAME
#include "sys/param.h"
#ifdef BSD4_4
#define TIME_NAME	"date"
#endif /* defined BSD4_4 */
#ifndef BSD4_4
#define TIME_NAME	""
#endif /* !defined BSD4_4 */
#endif /* !defined TIME_NAME */

#include "syslog.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "netdb.h"
#define TSPTYPES
#include "protocols/timed.h"

extern int		logwtmp();

#if HAVE_SETTIMEOFDAY == 1
#define settimeofday(t, tz) (settimeofday)(t)
#endif /* HAVE_SETTIMEOFDAY == 1 */

#ifdef TSP_SETDATE
<<<<<<< HEAD
static int netsettime(struct timeval);
=======
static bool netsettime(struct timeval);
>>>>>>> grandpa/master
#endif

#ifndef TSP_SETDATE
/*ARGSUSED*/
#endif /* !defined TSP_SETDATE */
static void
<<<<<<< HEAD
reset(const time_t newt, const int nflag)
=======
reset(time_t newt, bool nflag)
>>>>>>> grandpa/master
{
	register const char *	username;
	static struct timeval	tv;	/* static so tv_usec is 0 */

	username = getlogin();
	if (username == NULL || *username == '\0') /* single-user or no tty */
		username = "root";
	tv.tv_sec = newt;
#ifdef TSP_SETDATE
	if (nflag || !netsettime(tv))
#endif /* defined TSP_SETDATE */
	{
		/*
		** "old" entry is always written, for compatibility.
		*/
		logwtmp("|", TIME_NAME, "");
		if (settimeofday(&tv, NULL) == 0) {
			logwtmp("{", TIME_NAME, "");	/* } */
			syslog(LOG_AUTH | LOG_NOTICE, _("date set by %s"),
				username);
		} else	oops("settimeofday");
	}
<<<<<<< HEAD

	if (!argc)
		goto display;

	if (gtime(*argv)) {
		usage();
		retval = 1;
		goto display;
	}

	if (!uflag) {		/* convert to GMT assuming local time */
		tv.tv_sec += (long)tz.tz_minuteswest * SECS_PER_MIN;
				/* now fix up local daylight time */
		if (localtime((time_t *)&tv.tv_sec)->tm_isdst)
			tv.tv_sec -= SECS_PER_HOUR;
	}
	if (nflag || !netsettime(tv)) {
		logwtmp("|", "date", "");
		if (settimeofday(&tv, (struct timezone *)NULL)) {
			perror("settimeofday");
			retval = 1;
			goto display;
		}
		logwtmp("{", "date", "");
	}

	username = getlogin();
	if (!username || *username == '\0')	/* single-user or no tty */
		username = "root";
	syslog(LOG_AUTH | LOG_NOTICE, "date set by %s", username);

display:
	if (gettimeofday(&tv, (struct timezone *)NULL)) {
		perror("gettimeofday");
		exit(1);
	}
	if (uflag) {
		ap = asctime(gmtime((time_t *)&tv.tv_sec));
		tzn = "GMT";
	}
	else {
		struct tm *tp;

		tp = localtime((time_t *)&tv.tv_sec);
		ap = asctime(tp);
		tzn = tp->tm_zone;
	}
	printf("%.20s%s%s", ap, tzn, ap + 19);
	exit(retval);
}

/*
 * gtime --
 *	convert user's time into number of seconds
 */
static
gtime(ap)
	register char *ap;
=======
}

#endif /* !defined OLD_TIME */

static void
wildinput(const char *const item, const char *const value,
	  const char *const reason)
{
	fprintf(stderr,
		_("date: error: bad command line %s \"%s\", %s\n"),
		item, value, reason);
	usage();
}

static void
errensure(void)
{
	if (retval == EXIT_SUCCESS)
		retval = EXIT_FAILURE;
}

static const char * ATTRIBUTE_PURE
nondigit(register const char *cp)
{
	while (is_digit(*cp))
		++cp;
	return cp;
}

static void
usage(void)
{
	fprintf(stderr,
		       _("date: usage: date [-u] [-c] [-r seconds] [-n]"
			 " [-d dst] [-t min-west] [-a sss.fff]"
			 " [[yyyy]mmddhhmm[yyyy][.ss]] [+format]\n"));
	errensure();
	exit(retval);
}

static void
oops(const char *const string)
{
	int		e = errno;

	fprintf(stderr, _("date: error: "));
	errno = e;
	perror(string);
	errensure();
	display(NULL, time(NULL));
	exit(retval);
}

static void
display(const char *const format, time_t const now)
{
	struct tm *tmp;

	tmp = localtime(&now);
	if (!tmp) {
		fprintf(stderr,
			_("date: error: time out of range\n"));
		errensure();
		return;
	}
	timeout(stdout, format ? format : "%+", tmp);
	putchar('\n');
	fflush(stdout);
	fflush(stderr);
	if (ferror(stdout) || ferror(stderr)) {
		fprintf(stderr,
			_("date: error: couldn't write results\n"));
		errensure();
	}
}

#define INCR	1024

static void
timeout(FILE *const fp, const char *const format, const struct tm *tmp)
{
	char *	cp;
	size_t	result;
	size_t	size;
	struct tm tm;

	if (*format == '\0')
		return;
	if (!tmp) {
		fprintf(stderr, _("date: error: time out of range\n"));
		errensure();
		return;
	}
	tm = *tmp;
	tmp = &tm;
	size = INCR;
	cp = malloc(size);
	for ( ; ; ) {
		if (cp == NULL) {
			fprintf(stderr,
				_("date: error: can't get memory\n"));
			errensure();
			exit(retval);
		}
		cp[0] = '\1';
		result = strftime(cp, size, format, tmp);
		if (result != 0 || cp[0] == '\0')
			break;
		size += INCR;
		cp = realloc(cp, size);
	}
	fwrite(cp, 1, result, fp);
	free(cp);
}

static bool
sametm(register const struct tm *const atmp,
       register const struct tm *const btmp)
{
	return atmp->tm_year == btmp->tm_year &&
		atmp->tm_mon == btmp->tm_mon &&
		atmp->tm_mday == btmp->tm_mday &&
		atmp->tm_hour == btmp->tm_hour &&
		atmp->tm_min == btmp->tm_min &&
		atmp->tm_sec == btmp->tm_sec;
}

/*
** convert --
**	convert user's input into a time_t.
*/

#define ATOI2(ar)	(ar[0] - '0') * 10 + (ar[1] - '0'); ar += 2;

static time_t
convert(char const *value, bool dousg, time_t t)
{
	register const char *	cp;
	register const char *	dotp;
	register int	cent, year_in_cent, month, hour, day, mins, secs;
	struct tm	tm, outtm, *tmp;
	time_t		outt;

	tmp = localtime(&t);
	if (!tmp)
		return -1;
	tm = *tmp;
#define DIVISOR	100
	year_in_cent = tm.tm_year % DIVISOR + TM_YEAR_BASE % DIVISOR;
	cent = tm.tm_year / DIVISOR + TM_YEAR_BASE / DIVISOR +
		year_in_cent / DIVISOR;
	year_in_cent %= DIVISOR;
	if (year_in_cent < 0) {
		year_in_cent += DIVISOR;
		--cent;
	}
	month = tm.tm_mon + 1;
	day = tm.tm_mday;
	hour = tm.tm_hour;
	mins = tm.tm_min;
	secs = 0;

	dotp = strchr(value, '.');
	for (cp = value; *cp != '\0'; ++cp)
		if (!is_digit(*cp) && cp != dotp)
			wildinput(_("time"), value, _("contains a nondigit"));

	if (dotp == NULL)
		dotp = strchr(value, '\0');
	else {
		cp = dotp + 1;
		if (strlen(cp) != 2)
			wildinput(_("time"), value,
				_("seconds part is not two digits"));
		secs = ATOI2(cp);
	}

	cp = value;
	switch (dotp - cp) {
		default:
			wildinput(_("time"), value,
				_("main part is wrong length"));
		case 12:
			if (!dousg) {
				cent = ATOI2(cp);
				year_in_cent = ATOI2(cp);
			}
			month = ATOI2(cp);
			day = ATOI2(cp);
			hour = ATOI2(cp);
			mins = ATOI2(cp);
			if (dousg) {
				cent = ATOI2(cp);
				year_in_cent = ATOI2(cp);
			}
			break;
		case 8:	/* mmddhhmm */
			month = ATOI2(cp);
			/* fall through to. . . */
		case 6:	/* ddhhmm */
			day = ATOI2(cp);
			/* fall through to. . . */
		case 4:	/* hhmm */
			hour = ATOI2(cp);
			mins = ATOI2(cp);
			break;
		case 10:
			if (!dousg) {
				year_in_cent = ATOI2(cp);
			}
			month = ATOI2(cp);
			day = ATOI2(cp);
			hour = ATOI2(cp);
			mins = ATOI2(cp);
			if (dousg) {
				year_in_cent = ATOI2(cp);
			}
			break;
	}

	tm.tm_year = cent * 100 + year_in_cent - TM_YEAR_BASE;
	tm.tm_mon = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = mins;
	tm.tm_sec = secs;
	tm.tm_isdst = -1;
	outtm = tm;
	outt = mktime(&outtm);
	return sametm(&tm, &outtm) ? outt : -1;
}

/*
** Code from here on out is either based on code provided by UCB
** or is only called just before the program exits.
*/

/*
** Check for iffy input.
*/

static void
checkfinal(char const *value, bool didusg, time_t t, time_t oldnow)
>>>>>>> grandpa/master
{
	time_t		othert;
	struct tm	tm, *tmp;
	struct tm	othertm;
	register int	pass, offset;

	/*
	** See if there's both a USG and a BSD interpretation.
	*/
	othert = convert(value, !didusg, oldnow);
	if (othert != -1 && othert != t)
		iffy(t, othert, value, _("year could be at start or end"));
	/*
	** See if there's both a DST and a STD version.
	*/
	tmp = localtime(&t);
	if (!tmp)
		iffy(t, othert, value, _("time out of range"));
	othertm = tm = *tmp;
	othertm.tm_isdst = !tm.tm_isdst;
	othert = mktime(&othertm);
	if (othert != -1 && othertm.tm_isdst != tm.tm_isdst &&
		sametm(&tm, &othertm))
			iffy(t, othert, value,
			    _("both standard and summer time versions exist"));
/*
** Final check.
**
** If a jurisdiction shifts time *without* shifting whether time is
** summer or standard (as Hawaii, the United Kingdom, and Saudi Arabia
** have done), routine checks for iffy times may not work.
** So we perform this final check, deferring it until after the time has
** been set; it may take a while, and we don't want to introduce an unnecessary
** lag between the time the user enters their command and the time that
** stime/settimeofday is called.
**
** We just check nearby times to see if any have the same representation
** as the time that convert returned.  We work our way out from the center
** for quick response in solar time situations.  We only handle common cases:
** offsets of at most a minute, and offsets of exact numbers of minutes
** and at most an hour.
*/
	for (offset = 1; offset <= 60; ++offset)
		for (pass = 1; pass <= 4; ++pass) {
			if (pass == 1)
				othert = t + offset;
			else if (pass == 2)
				othert = t - offset;
			else if (pass == 3)
				othert = t + 60 * offset;
			else	othert = t - 60 * offset;
			tmp = localtime(&othert);
			if (!tmp)
				iffy(t, othert, value,
					_("time out of range"));
			othertm = *tmp;
			if (sametm(&tm, &othertm))
				iffy(t, othert, value,
					_("multiple matching times exist"));
		}
<<<<<<< HEAD
		if (!isdigit(*C))
			return(-1);
	}

	L = localtime((time_t *)&tv.tv_sec);
	year = L->tm_year;			/* defaults */
	month = L->tm_mon + 1;
	day = L->tm_mday;

	switch ((int)(C - ap)) {		/* length */
		case 10:			/* yymmddhhmm */
			year = ATOI2(ap);
		case 8:				/* mmddhhmm */
			month = ATOI2(ap);
		case 6:				/* ddhhmm */
			day = ATOI2(ap);
		case 4:				/* hhmm */
			hour = ATOI2(ap);
			mins = ATOI2(ap);
			break;
		default:
			return(1);
	}

	if (*ap || month < 1 || month > 12 || day < 1 || day > 31 ||
	     mins < 0 || mins > 59 || secs < 0 || secs > 59)
		return(1);
	if (hour == 24) {
		++day;
		hour = 0;
	}
	else if (hour < 0 || hour > 23)
		return(1);

	tv.tv_sec = 0;
	year += TM_YEAR_BASE;
	if (isleap(year) && month > 2)
		++tv.tv_sec;
	for (--year;year >= EPOCH_YEAR;--year)
		tv.tv_sec += isleap(year) ? DAYS_PER_LYEAR : DAYS_PER_NYEAR;
	while (--month)
		tv.tv_sec += dmsize[month];
	tv.tv_sec += day - 1;
	tv.tv_sec = HOURS_PER_DAY * tv.tv_sec + hour;
	tv.tv_sec = MINS_PER_HOUR * tv.tv_sec + mins;
	tv.tv_sec = SECS_PER_MIN * tv.tv_sec + secs;
	return(0);
}

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define TSPTYPES
#include <protocols/timed.h>

#define	WAITACK		2	/* seconds */
#define	WAITDATEACK	5	/* seconds */

extern	int errno;
/*
 * Set the date in the machines controlled by timedaemons
 * by communicating the new date to the local timedaemon. 
 * If the timedaemon is in the master state, it performs the
 * correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Returns 1 on success, 0 on failure.
 */
netsettime(ntv)
	struct timeval ntv;
{
	int s, length, port, timed_ack, found, err;
	long waittime;
	fd_set ready;
	char hostname[MAXHOSTNAMELEN];
=======
}

static void
iffy(const time_t thist, const time_t thatt,
	const char * const value, const char * const reason)
{
	struct tm *tmp;
	bool dst;

	fprintf(stderr, _("date: warning: ambiguous time \"%s\", %s.\n"),
		value, reason);
	tmp = gmtime(&thist);
	/*
	** Avoid running afoul of SCCS!
	*/
	timeout(stderr, _("Time was set as if you used\n\tdate -u %m%d%H\
%M\
%Y.%S\n"), tmp);
	tmp = localtime(&thist);
	dst = tmp && tmp->tm_isdst;
	timeout(stderr, _("to get %c"), tmp);
	fprintf(stderr, _(" (%s).  Use\n"),
		dst ? _("summer time") : _("standard time"));
	tmp = gmtime(&thatt);
	timeout(stderr, _("\tdate -u %m%d%H\
%M\
%Y.%S\n"), tmp);
	tmp = localtime(&thatt);
	dst = tmp && tmp->tm_isdst;
	timeout(stderr, _("to get %c"), tmp);
	fprintf(stderr, _(" (%s).\n"),
		dst ? _("summer time") : _("standard time"));
	errensure();
	exit(retval);
}

#ifdef TSP_SETDATE
#define WAITACK		2	/* seconds */
#define WAITDATEACK	5	/* seconds */

/*
 * Set the date in the machines controlled by timedaemons
 * by communicating the new date to the local timedaemon.
 * If the timedaemon is in the master state, it performs the
 * correction on all slaves.  If it is in the slave state, it
 * notifies the master that a correction is needed.
 * Return true on success.
 */
static bool
netsettime(struct timeval ntv)
{
	int s, length, port, timed_ack, found, err, waittime;
	fd_set ready;
>>>>>>> grandpa/master
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	sp = getservbyname("timed", "udp");
<<<<<<< HEAD
	if (sp == 0) {
		fputs("udp/timed: unknown service\n", stderr);
		retval = 2;
		return (0);
	}
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
=======
	if (! sp) {
		fputs(_("udp/timed: unknown service\n"), stderr);
		retval = 2;
		return false;
	}
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl(INADDR_ANY);
>>>>>>> grandpa/master
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
<<<<<<< HEAD
		sin.sin_port = htons((u_short)port);
=======
		sin.sin_port = htons(port);
>>>>>>> grandpa/master
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
<<<<<<< HEAD
		fputs("date: All ports in use\n", stderr);
=======
		fputs(_("date: All ports in use\n"), stderr);
>>>>>>> grandpa/master
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
<<<<<<< HEAD
	if (gethostname(hostname, sizeof (hostname))) {
		perror("gethostname");
		goto bad;
	}
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)ntv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)ntv.tv_usec);
=======
	msg.tsp_name[sizeof msg.tsp_name - 1] = '\0';
	if (gethostname(msg.tsp_name, sizeof msg.tsp_name) != 0) {
		perror("gethostname");
		goto bad;
	}
	if (msg.tsp_name[sizeof msg.tsp_name - 1]) {
		fprintf(stderr, "hostname too long\n");
		goto bad;
	}
	msg.tsp_seq = htons(0);
	msg.tsp_time.tv_sec = htonl(ntv.tv_sec);
	msg.tsp_time.tv_usec = htonl(ntv.tv_usec);
>>>>>>> grandpa/master
	length = sizeof (struct sockaddr_in);
	if (connect(s, &dest, length) < 0) {
		perror("date: connect");
		goto bad;
	}
	if (send(s, (char *)&msg, sizeof (struct tsp), 0) < 0) {
		if (errno != ECONNREFUSED)
			perror("date: send");
		goto bad;
	}
	timed_ack = -1;
	waittime = WAITACK;
loop:
	tout.tv_sec = waittime;
	tout.tv_usec = 0;
	FD_ZERO(&ready);
	FD_SET(s, &ready);
<<<<<<< HEAD
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
=======
	found = select(FD_SETSIZE, &ready, NULL, NULL, &tout);
	length = sizeof err;
>>>>>>> grandpa/master
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
<<<<<<< HEAD
			perror("date: send (delayed error)");
=======
			perror(_("date: send (delayed error)"));
>>>>>>> grandpa/master
		goto bad;
	}
	if (found > 0 && FD_ISSET(s, &ready)) {
		length = sizeof (struct sockaddr_in);
		if (recvfrom(s, (char *)&msg, sizeof (struct tsp), 0, &from,
		    &length) < 0) {
			if (errno != ECONNREFUSED)
				perror("date: recvfrom");
			goto bad;
		}
		msg.tsp_seq = ntohs(msg.tsp_seq);
		msg.tsp_time.tv_sec = ntohl(msg.tsp_time.tv_sec);
		msg.tsp_time.tv_usec = ntohl(msg.tsp_time.tv_usec);
		switch (msg.tsp_type) {

		case TSP_ACK:
			timed_ack = TSP_ACK;
			waittime = WAITDATEACK;
			goto loop;

		case TSP_DATEACK:
<<<<<<< HEAD
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    "date: Wrong ack received from timed: %s\n", 
			    tsptype[msg.tsp_type]);
=======
			lose(s);
			return true;

		default:
			fprintf(stderr,
				_("date: Wrong ack received from timed: %s\n"),
				tsptype[msg.tsp_type]);
>>>>>>> grandpa/master
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
<<<<<<< HEAD
		fputs("date: Can't reach time daemon, time set locally.\n",
		    stderr);
bad:
	(void)close(s);
	retval = 2;
	return (0);
}

usage()
{
	fputs("usage: date [-nu] [-d dst] [-t minutes_west] [yymmddhhmm[.ss]]\n", stderr);
}
=======
		fputs(_("date: Can't reach time daemon, time set locally.\n"),
			stderr);
bad:
	lose(s);
	retval = 2;
	return false;
}
#endif /* defined TSP_SETDATE */
>>>>>>> grandpa/master
