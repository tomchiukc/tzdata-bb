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
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

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

main(argc, argv)
	int argc;
	char **argv;
{
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

	if (gettimeofday(&tv, &tz)) {
		perror("gettimeofday");
		exit(1);
	}

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
	struct timeval tout;
	struct servent *sp;
	struct tsp msg;
	struct sockaddr_in sin, dest, from;

	sp = getservbyname("timed", "udp");
	if (sp == 0) {
		fputs("udp/timed: unknown service\n", stderr);
		retval = 2;
		return (0);
	}
	dest.sin_port = sp->s_port;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl((u_long)INADDR_ANY);
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		if (errno != EPROTONOSUPPORT)
			perror("date: socket");
		goto bad;
	}
	bzero((char *)&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	for (port = IPPORT_RESERVED - 1; port > IPPORT_RESERVED / 2; port--) {
		sin.sin_port = htons((u_short)port);
		if (bind(s, (struct sockaddr *)&sin, sizeof (sin)) >= 0)
			break;
		if (errno != EADDRINUSE) {
			if (errno != EADDRNOTAVAIL)
				perror("date: bind");
			goto bad;
		}
	}
	if (port == IPPORT_RESERVED / 2) {
		fputs("date: All ports in use\n", stderr);
		goto bad;
	}
	msg.tsp_type = TSP_SETDATE;
	msg.tsp_vers = TSPVERSION;
	if (gethostname(hostname, sizeof (hostname))) {
		perror("gethostname");
		goto bad;
	}
	(void) strncpy(msg.tsp_name, hostname, sizeof (hostname));
	msg.tsp_seq = htons((u_short)0);
	msg.tsp_time.tv_sec = htonl((u_long)ntv.tv_sec);
	msg.tsp_time.tv_usec = htonl((u_long)ntv.tv_usec);
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
	found = select(FD_SETSIZE, &ready, (fd_set *)0, (fd_set *)0, &tout);
	length = sizeof(err);
	if (getsockopt(s, SOL_SOCKET, SO_ERROR, (char *)&err, &length) == 0
	    && err) {
		errno = err;
		if (errno != ECONNREFUSED)
			perror("date: send (delayed error)");
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
			(void)close(s);
			return (1);

		default:
			fprintf(stderr,
			    "date: Wrong ack received from timed: %s\n", 
			    tsptype[msg.tsp_type]);
			timed_ack = -1;
			break;
		}
	}
	if (timed_ack == -1)
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
