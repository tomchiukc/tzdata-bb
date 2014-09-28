/*
** This file is in the public domain, so clarified as of
** 2009-05-17 by Arthur David Olson.
*/

#include "version.h"

/*
** This code has been made independent of the rest of the time
** conversion package to increase confidence in the verification it provides.
** You can use this code to help in verifying other implementations.
** To do this, compile with -DUSE_LTZ=0 and link without the tz library.
*/

#ifndef NETBSD_INSPIRED
# define NETBSD_INSPIRED 1
#endif
#ifndef USE_LTZ
# define USE_LTZ 1
#endif

#if USE_LTZ
# include "private.h"
#endif

/* Enable tm_gmtoff and tm_zone on GNUish systems.  */
#define _GNU_SOURCE 1
/* Enable strtoimax on Solaris 10.  */
#define __EXTENSIONS__ 1

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

/* Pre-C99 GCC compilers define __LONG_LONG_MAX__ instead of LLONG_MAX.  */
#if !defined LLONG_MAX && defined __LONG_LONG_MAX__
# define LLONG_MAX __LONG_LONG_MAX__
#endif

#ifndef INTMAX_MAX
# ifdef LLONG_MAX
typedef long long intmax_t;
#  define strtoimax strtoll
#  define INTMAX_MAX LLONG_MAX
# else
typedef long intmax_t;
#  define strtoimax strtol
#  define INTMAX_MAX LONG_MAX
# endif
#endif

#ifndef PRIdMAX
# if INTMAX_MAX == LLONG_MAX
#  define PRIdMAX "lld"
# else
#  define PRIdMAX "ld"
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

#ifndef HAVE_LOCALTIME_R
# define HAVE_LOCALTIME_R 1
#endif

#ifndef HAVE_LOCALTIME_RZ
# ifdef TM_ZONE
#  define HAVE_LOCALTIME_RZ (NETBSD_INSPIRED && USE_LTZ)
# else
#  define HAVE_LOCALTIME_RZ 0
# endif
#endif

#ifndef HAVE_TZSET
# define HAVE_TZSET 1
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

#include "version.h"

#ifdef OBJECTID
static char	sccsid[] = "%W%";
#endif

#include "timezone.h"

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

#ifndef TZ_DOMAIN
#define TZ_DOMAIN "tz"
#endif /* !defined TZ_DOMAIN */

extern char **	environ;
extern int	getopt(int argc, char * const argv[],
			const char * options);
extern char *	optarg;
extern int	optind;
extern char *	sprintf();
extern long	time();

main(argc, argv)
int	argc;
char *	argv[];
{
	register FILE *	fp;
	register int	i, j, c;
	int		vflag;
	long		now;
	struct tzinfo 	t;
	char		buf[BUFSIZ];

	INITIALIZE(cutlotime);
	INITIALIZE(cuthitime);
#if HAVE_GETTEXT
	(void) setlocale(LC_ALL, "");
#ifdef TZ_DOMAINDIR
	(void) bindtextdomain(TZ_DOMAIN, TZ_DOMAINDIR);
#endif /* defined TEXTDOMAINDIR */
	(void) textdomain(TZ_DOMAIN);
#endif /* HAVE_GETTEXT */
	progname = argv[0];
	for (i = 1; i < argc; ++i)
		if (strcmp(argv[i], "--version") == 0) {
			(void) printf("%s\n", TZVERSION);
			exit(EXIT_SUCCESS);
		} else if (strcmp(argv[i], "--help") == 0) {
			usage(stdout, EXIT_SUCCESS);
		}
	vflag = 0;
	while ((c = getopt(argc, argv, "v")) == 'v')
		vflag = 1;
	if (c != EOF || optind == argc - 1 && strcmp(argv[optind], "=") == 0) {
		(void) fprintf(stderr, "%s: usage is %s zonename ...\n",
			argv[0], argv[0]);
		exit(1);
	}
	(void) time(&now);
	for (i = optind; i < argc; ++i) {
		if (settz(argv[i]) != 0) {
(void) fprintf(stderr, "%s: wild result from settz(\"%s\")\n",
			argv[0], argv[i]);
			exit(1);
		}
		(void) printf("%s: %s", argv[i], newctime(&now));
		if (!vflag)
			continue;
		(void) sprintf(buf, "%s/%s", TZDIR, argv[i]);
		if ((fp = fopen(buf, "r")) == NULL &&
			(fp = fopen(argv[i], "r")) == NULL) {
(void) fprintf(stderr, "%s: wild result opening %s file\n",
					argv[0], argv[i]);
				exit(1);
		}
		if (fread((char *) &t, sizeof t, 1, fp) != 1) {
(void) fprintf(stderr, "%s: wild result reading %s file\n",
					argv[0], argv[i]);
				exit(1);
		}
		if (fclose(fp)) {
(void) fprintf(stderr, "%s: wild result closing %s file\n",
					argv[0], argv[i]);
				exit(1);
		}
		for (j = 0; j < t.tz_rulecnt; ++j)
			(void) printf("%s: %s", argv[i],
				newctime(&t.tz_times[j]));
	}
	return 0;
}
