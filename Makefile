# This file is in the public domain, so clarified as of
# 2009-05-17 by Arthur David Olson.

# Package name for the code distribution.
PACKAGE=	tzcode

# Version numbers of the code and data distributions.
VERSION=	2013f

# Email address for bug reports.
BUGEMAIL=	tz@iana.org

# Change the line below for your time zone (after finding the zone you want in
# the time zone files, or adding it to a time zone file).
# Alternately, if you discover you've got the wrong time zone, you can just
#	zic -l rightzone
# to correct things.
# Use the command
#	make zonenames
# to get a list of the values you can use for LOCALTIME.
LINTFLAGS=	-phbaaxc

CFLAGS=		-g -DFAKENAME=$(FAKENAME) -DFAKEOFF=$(FAKEOFF) \
			-DFAKEST='$(FAKEST)' -DFAKEDT='$(FAKEDT)'

ALL=		try tzdir/$(FAKENAME) fake

all:		$(ALL)

try:		try.o ctime.o
			$(CC) $(CFLAGS) try.o ctime.o $(LIBS) -o $@

tzdir/Eastern:	tzdir fake
		fake > $@

tzdir:
		mkdir tzdir

fake:		fake.o
			$(CC) $(CFLAGS) fake.o $(LIBS) -o $@

bundle:		Makefile timezone.h ctime.c try.c fake.c
			bundle Makefile timezone.h ctime.c try.c fake.c > bundle

Makefile timezone.h ctime.c try.c fake.c:
			sccs get $(REL) $(REV) $@

# If you "make INSTALL", the "date" command gets installed in. . .

BINDIR=		$(TOPDIR)/bin

# Manual pages go in subdirectories of. . .

MANDIR=		$(TOPDIR)/man

# Library functions are put in an archive in LIBDIR.

LIBDIR=		$(TOPDIR)/lib
TZLIB=		$(LIBDIR)/libtz.a

# If you always want time values interpreted as "seconds since the epoch
# (not counting leap seconds)", use
#	REDO=		posix_only
# below.  If you always want right time values interpreted as "seconds since
# the epoch" (counting leap seconds)", use
#	REDO=		right_only
# below.  If you want both sets of data available, with leap seconds not
# counted normally, use
#	REDO=		posix_right
# below.  If you want both sets of data available, with leap seconds counted
# normally, use
#	REDO=		right_posix
# below.
# POSIX mandates that leap seconds not be counted; for compatibility with it,
# use either "posix_only" or "posix_right".

REDO=		posix_right

# Since "." may not be in PATH...

YEARISTYPE=	./yearistype

# Non-default libraries needed to link.
# Add -lintl if you want to use 'gettext' on Solaris.
LDLIBS=

# Add the following to the end of the "CFLAGS=" line as needed.
#  -DBIG_BANG=-9999999LL if the Big Bang occurred at time -9999999 (see zic.c)
#  -DHAVE_ADJTIME=0 if 'adjtime' does not exist (SVR0?)
#  -DHAVE_DOS_FILE_NAMES if file names have drive specifiers etc. (MS-DOS)
#  -DHAVE_GETTEXT=1 if 'gettext' works (GNU, Linux, Solaris); also see LDLIBS
#  -DHAVE_INCOMPATIBLE_CTIME_R=1 if your system's time.h declares
#	ctime_r and asctime_r incompatibly with the POSIX standard (Solaris 8).
#  -DHAVE_SETTIMEOFDAY=0 if settimeofday does not exist (SVR0?)
#  -DHAVE_SETTIMEOFDAY=1 if settimeofday has just 1 arg (SVR4)
#  -DHAVE_SETTIMEOFDAY=2 if settimeofday uses 2nd arg (4.3BSD)
#  -DHAVE_SETTIMEOFDAY=3 if settimeofday ignores 2nd arg (4.4BSD)
#  -DHAVE_STDINT_H=1 if you have a pre-C99 compiler with "stdint.h"
#  -DHAVE_SYMLINK=0 if your system lacks the symlink function
#  -DHAVE_SYS_STAT_H=0 if your compiler lacks a "sys/stat.h"
#  -DHAVE_SYS_WAIT_H=0 if your compiler lacks a "sys/wait.h"
#  -DLOCALE_HOME=\"path\" if locales are in "path", not "/usr/lib/locale"
#  -DHAVE_UNISTD_H=0 if your compiler lacks a "unistd.h" (Microsoft C++ 7?)
#  -DHAVE_UTMPX_H=1 if your compiler has a "utmpx.h"
#  -DTZDEFRULESTRING=\",date/time,date/time\" to default to the specified
#	DST transitions if the time zone files cannot be accessed
#  -DTZ_DOMAIN=\"foo\" to use "foo" for gettext domain name; default is "tz"
#  -DTZ_DOMAINDIR=\"/path\" to use "/path" for gettext directory;
#	the default is system-supplied, typically "/usr/lib/locale"
#  $(GCC_DEBUG_FLAGS) if you are using GCC and want lots of checking
#  -DNO_RUN_TIME_WARNINGS_ABOUT_YEAR_2000_PROBLEMS_THANK_YOU=1
#	if you do not want run time warnings about formats that may cause
#	year 2000 grief
#  -DZIC_MAX_ABBR_LEN_WO_WARN=3
#	(or some other number) to set the maximum time zone abbreviation length
#	that zic will accept without a warning (the default is 6)
GCC_DEBUG_FLAGS = -Dlint -g -O3 -fno-common \
	-Wall -Wcast-qual -Wconversion -Wmissing-prototypes \
	-Wnested-externs -Wpointer-arith -Wshadow \
	-Wtraditional # -Wstrict-prototypes -Wwrite-strings
#
# If you want to use System V compatibility code, add
#	-DUSG_COMPAT
# to the end of the "CFLAGS=" line.  This arrange for "timezone" and "daylight"
# variables to be kept up-to-date by the time conversion functions.  Neither
# "timezone" nor "daylight" is described in X3J11's work.
#
# If your system has a "GMT offset" field in its "struct tm"s
# (or if you decide to add such a field in your system's "time.h" file),
# add the name to a define such as
#	-DTM_GMTOFF=tm_gmtoff
# or
#	-DTM_GMTOFF=_tm_gmtoff
# to the end of the "CFLAGS=" line.
# Neither tm_gmtoff nor _tm_gmtoff is described in X3J11's work;
# in its work, use of "tm_gmtoff" is described as non-conforming.
# Both Linux and BSD have done the equivalent of defining TM_GMTOFF in
# their recent releases.
#
# If your system has a "zone abbreviation" field in its "struct tm"s
# (or if you decide to add such a field in your system's "time.h" file),
# add the name to a define such as
#	-DTM_ZONE=tm_zone
# or
#	-DTM_ZONE=_tm_zone
# to the end of the "CFLAGS=" line.
# Neither tm_zone nor _tm_zone is described in X3J11's work;
# in its work, use of "tm_zone" is described as non-conforming.
# Both UCB and Sun have done the equivalent of defining TM_ZONE in
# their recent releases.
#
# If you want functions that were inspired by early versions of X3J11's work,
# add
#	-DSTD_INSPIRED
# to the end of the "CFLAGS=" line.  This arranges for the functions
# "tzsetwall", "offtime", "timelocal", "timegm", "timeoff",
# "posix2time", and "time2posix" to be added to the time conversion library.
# "tzsetwall" is like "tzset" except that it arranges for local wall clock
# time (rather than the time specified in the TZ environment variable)
# to be used.
# "offtime" is like "gmtime" except that it accepts a second (long) argument
# that gives an offset to add to the time_t when converting it.
# "timelocal" is equivalent to "mktime".
# "timegm" is like "timelocal" except that it turns a struct tm into
# a time_t using UTC (rather than local time as "timelocal" does).
# "timeoff" is like "timegm" except that it accepts a second (long) argument
# that gives an offset to use when converting to a time_t.
# "posix2time" and "time2posix" are described in an included manual page.
# X3J11's work does not describe any of these functions.
# Sun has provided "tzsetwall", "timelocal", and "timegm" in SunOS 4.0.
# These functions may well disappear in future releases of the time
# conversion package.
#
# If you want Source Code Control System ID's left out of object modules, add
#	-DNOID
# to the end of the "CFLAGS=" line.
#
# If you'll never want to handle solar-time-based time zones, add
#	-DNOSOLAR
# to the end of the "CFLAGS=" line
# (and comment out the "SDATA=" line below).
# This reduces (slightly) the run-time data-space requirements of
# the time conversion functions; it may reduce the acceptability of your system
# to folks in oil- and cash-rich places.
#
# If you want to allocate state structures in localtime, add
#	-DALL_STATE
# to the end of the "CFLAGS=" line.  Storage is obtained by calling malloc.
#
# If you want an "altzone" variable (a la System V Release 3.1), add
#	-DALTZONE
# to the end of the "CFLAGS=" line.
# This variable is not described in X3J11's work.
#
# If you want a "gtime" function (a la MACH), add
#	-DCMUCS
# to the end of the "CFLAGS=" line
# This function is not described in X3J11's work.
#
# NIST-PCTS:151-2, Version 1.4, (1993-12-03) is a test suite put
# out by the National Institute of Standards and Technology
# which claims to test C and Posix conformance.  If you want to pass PCTS, add
#	-DPCTS
# to the end of the "CFLAGS=" line.
#
# If you want strict compliance with XPG4 as of 1994-04-09, add
#	-DXPG4_1994_04_09
# to the end of the "CFLAGS=" line.  This causes "strftime" to always return
# 53 as a week number (rather than 52 or 53) for those days in January that
# before the first Monday in January when a "%V" format is used and January 1
# falls on a Friday, Saturday, or Sunday.

CFLAGS=

# Linker flags.  Default to $(LFLAGS) for backwards compatibility
# to tzcode2012h and earlier.

LDFLAGS=	$(LFLAGS)

zic=		./zic
ZIC=		$(zic) $(ZFLAGS)

ZFLAGS=

# The name of a Posix-compliant 'awk' on your system.
AWK=		awk

# The full path name of a Posix-compliant shell, preferably one that supports
# the Korn shell's 'select' statement as an extension.
# These days, Bash is the most popular.
# It should be OK to set this to /bin/sh, on platforms where /bin/sh
# lacks 'select' or doesn't completely conform to Posix, but /bin/bash
# is typically nicer if it works.
KSHELL=		/bin/bash

# The path where SGML DTDs are kept and the catalog file(s) to use when
# validating.  The default is appropriate for Ubuntu 13.10.
SGML_TOPDIR= /usr
SGML_SEARCH_PATH= $(SGML_TOPDIR)/share/xml/xhtml/schema/dtd/REC-html401-19991224

# The catalog file(s) to use when validating.
SGML_CATALOG_FILES= HTML4.cat

# The name, arguments and environment of a program to validate your web pages.
# See <http://www.jclark.com/sp/> for a validator, and
# <http://validator.w3.org/source/> for a validation library.
VALIDATE = nsgmls
VALIDATE_FLAGS = -s -B -wall -wno-unused-param
VALIDATE_ENV = \
  SGML_CATALOG_FILES=$(SGML_CATALOG_FILES) \
  SGML_SEARCH_PATH=$(SGML_SEARCH_PATH) \
  SP_CHARSET_FIXED=YES \
  SP_ENCODING=UTF-8

# Flags to give 'tar' when making a distribution.
# Try to use flags appropriate for GNU tar.
GNUTARFLAGS=	--numeric-owner --owner=0 --group=0 --mode=go+u,go-w
TARFLAGS=	`if tar $(GNUTARFLAGS) --version >/dev/null 2>&1; \
		 then echo $(GNUTARFLAGS); \
		 else :; \
		 fi`

# Flags to give 'gzip' when making a distribution.
GZIPFLAGS=	-9n

###############################################################################

cc=		cc
CC=		$(cc) -DTZDIR=\"$(TZDIR)\"

TZCSRCS=	zic.c localtime.c asctime.c scheck.c ialloc.c
TZCOBJS=	zic.o localtime.o asctime.o scheck.o ialloc.o
TZDSRCS=	zdump.c localtime.c ialloc.c
TZDOBJS=	zdump.o localtime.o ialloc.o
DATESRCS=	date.c localtime.c strftime.c asctime.c
DATEOBJS=	date.o localtime.o strftime.o asctime.o
LIBSRCS=	localtime.c asctime.c difftime.c
LIBOBJS=	localtime.o asctime.o difftime.o
HEADERS=	tzfile.h private.h
NONLIBSRCS=	zic.c zdump.c scheck.c ialloc.c
NEWUCBSRCS=	date.c strftime.c
SOURCES=	$(HEADERS) $(LIBSRCS) $(NONLIBSRCS) $(NEWUCBSRCS) tzselect.ksh
MANS=		newctime.3 newstrftime.3 newtzset.3 time2posix.3 \
			tzfile.5 tzselect.8 zic.8 zdump.8
COMMON=		Makefile README
DOCS=		NEWS Theory $(MANS) date.1
PRIMARY_YDATA=	africa antarctica asia australasia \
		europe northamerica southamerica
YDATA=		$(PRIMARY_YDATA) pacificnew etcetera backward
NDATA=		systemv factory
TDATA=		$(YDATA) $(NDATA)
TIMETABLES=	time.tab zone.tab
TABDATA=	iso3166.tab $(TIMETABLES) leapseconds
LEAP_DEPS=	leapseconds.awk leap-seconds.list
DATA=		$(YDATA) $(NDATA) $(TABDATA) \
			$(LEAP_DEPS) yearistype.sh
AWK_SCRIPTS=	checktab.awk leapseconds.awk
MISC=		$(AWK_SCRIPTS) workman.sh zoneinfo2tdf.pl
ENCHILADA=	$(COMMON) $(DOCS) $(SOURCES) $(DATA) $(MISC)

# And for the benefit of csh users on systems that assume the user
# shell should be used to handle commands in Makefiles. . .

SHELL=		/bin/sh

all:		tzselect zic zdump $(LIBOBJS)

ALL:		all date

install:	all $(DATA) $(REDO) $(TZLIB) $(MANS) $(TABDATA)
		$(ZIC) -y $(YEARISTYPE) \
			-d $(DESTDIR)$(TZDIR) -l $(LOCALTIME) -p $(POSIXRULES)
		cp -f iso3166.tab $(TIMETABLES) $(DESTDIR)$(TZDIR)/.
		cp tzselect zic zdump $(DESTDIR)$(ETCDIR)/.
		cp libtz.a $(DESTDIR)$(LIBDIR)/.
		$(RANLIB) $(DESTDIR)$(LIBDIR)/libtz.a
		cp -f newctime.3 newtzset.3 $(DESTDIR)$(MANDIR)/man3/.
		cp -f tzfile.5 $(DESTDIR)$(MANDIR)/man5/.
		cp -f tzselect.8 zdump.8 zic.8 $(DESTDIR)$(MANDIR)/man8/.

INSTALL:	ALL install date.1
		-mkdir $(TOPDIR) $(BINDIR)
		cp date $(BINDIR)/.
		-mkdir $(TOPDIR) $(MANDIR) $(MANDIR)/man1
		-rm -f $(MANDIR)/man1/date.1
		cp date.1 $(MANDIR)/man1/.

version.h:
		echo >$@ \
		  'static char const TZVERSION[]="tz$(VERSION)";'

zdump:		$(TZDOBJS)
		$(CC) $(CFLAGS) $(LFLAGS) $(TZDOBJS) $(LDLIBS) -o $@

zic:		$(TZCOBJS) yearistype
		$(CC) $(CFLAGS) $(LFLAGS) $(TZCOBJS) $(LDLIBS) -o $@

yearistype:	yearistype.sh
		cp yearistype.sh yearistype
		chmod +x yearistype

leapseconds:	leapseconds.awk leap-seconds.list
		$(AWK) -f leapseconds.awk leap-seconds.list >$@

posix_only:	zic $(TDATA)
		$(ZIC) -y $(YEARISTYPE) -d $(TZDIR) -L /dev/null $(TDATA)

right_only:	zic leapseconds $(TDATA)
		$(ZIC) -y $(YEARISTYPE) -d $(TZDIR) -L leapseconds $(TDATA)

# In earlier versions of this makefile, the other two directories were
# subdirectories of $(TZDIR).  However, this led to configuration errors.
# For example, with posix_right under the earlier scheme,
# TZ='right/Australia/Adelaide' got you localtime with leap seconds,
# but gmtime without leap seconds, which led to problems with applications
# like sendmail that subtract gmtime from localtime.
# Therefore, the other two directories are now siblings of $(TZDIR).
# You must replace all of $(TZDIR) to switch from not using leap seconds
# to using them, or vice versa.
right_posix:	right_only leapseconds
		rm -fr $(TZDIR)-leaps
		ln -s $(TZDIR_BASENAME) $(TZDIR)-leaps || \
		  $(ZIC) -y $(YEARISTYPE) \
			-d $(TZDIR)-leaps -L leapseconds $(TDATA)
		$(ZIC) -y $(YEARISTYPE) -d $(TZDIR)-posix -L /dev/null $(TDATA)

posix_right:	posix_only leapseconds
		rm -fr $(TZDIR)-posix
		ln -s $(TZDIR_BASENAME) $(TZDIR)-posix || \
		  $(ZIC) -y $(YEARISTYPE) \
			-d $(TZDIR)-posix -L /dev/null $(TDATA)
		$(ZIC) -y $(YEARISTYPE) \
			-d $(TZDIR)-leaps -L leapseconds $(TDATA)

posix_right:	posix_only other_two

right_posix:	right_only other_two

zones:		$(REDO)

$(TZLIB):	$(LIBOBJS)
		-mkdir $(TOPDIR) $(LIBDIR)
		ar ru $@ $(LIBOBJS)
		if [ -x /usr/ucb/ranlib -o -x /usr/bin/ranlib ] ; \
			then ranlib $@ ; fi

date:		$(DATEOBJS)
		$(CC) $(CFLAGS) date.o localtime.o asctime.o strftime.o \
			$(LDLIBS) -lc -o $@

tzselect:	tzselect.ksh
		sed \
			-e 's|AWK=[^}]*|AWK=$(AWK)|g' \
			-e 's|TZDIR=[^}]*|TZDIR=$(TZDIR)|' \
			-e 's|\(TZVERSION\)=.*|\1=tz$(VERSION)|' \
			<$? >$@
		chmod +x $@

check:		check_character_set check_tables check_web

check_character_set: $(ENCHILADA)
		LC_ALL=en_US.utf8 && export LC_ALL && \
		sharp='#' && \
		! grep -Env $(SAFE_LINE) $(MANS) date.1 $(MANTXTS) \
			$(MISC) $(SOURCES) $(WEB_PAGES) && \
		! grep -Env $(SAFE_SHARP_LINE) $(YDATA) $(NDATA) iso3166.tab \
			zone.tab leapseconds $(LEAP_DEPS) yearistype.sh && \
		test $$(grep -Ecv $(SAFE_SHARP_LINE) Makefile) -eq 1 && \
		! grep -Env $(NONSYM_LINE) README NEWS Theory $(MANS) date.1 \
			time.tab && \
		! grep -Env $(VALID_LINE) $(ENCHILADA)

check_tables:	checktab.awk $(PRIMARY_YDATA) $(TIMETABLES)
		for tab in $(TIMETABLES); do \
		  $(AWK) -f checktab.awk -v zone_table=$$tab $(PRIMARY_YDATA) \
		    || exit; \
		done

check_web:	$(WEB_PAGES)
		$(VALIDATE_ENV) $(VALIDATE) $(VALIDATE_FLAGS) $(WEB_PAGES)

clean:
		rm -f core *.o *.out \
		  date leapseconds tzselect version.h zdump zic yearistype
clean:		clean_misc
		rm -f -r tzpublic

maintainer-clean: clean
		@echo 'This command is intended for maintainers to use; it'
		@echo 'deletes files that may need special tools to rebuild.'
		rm -f *.[1-8].txt tzcode*.tar.gz tzdata*.tar.gz

names:
		@echo $(ENCHILADA)

public:		check check_public check_time_t_alternatives \
		tarballs signatures

date.1.txt:	date.1
newctime.3.txt:	newctime.3
newstrftime.3.txt: newstrftime.3
newtzset.3.txt:	newtzset.3
time2posix.3.txt: time2posix.3
tzfile.5.txt:	tzfile.5
tzselect.8.txt:	tzselect.8
zdump.8.txt:	zdump.8
zic.8.txt:	zic.8

$(MANTXTS):	workman.sh
		LC_ALL=en_US.utf8 sh workman.sh `expr $@ : '\(.*\)\.txt$$'` >$@

# Set the time stamps to those of the git repository, if available,
# and if the files have not changed since then.
# This uses GNU 'touch' syntax 'touch -d@N FILE',
# where N is the number of seconds since 1970.
# If git or GNU 'touch' is absent, do nothing.
set-timestamps:
		-TZ=UTC0 && export TZ && files=`git ls-files` && \
		touch -d @1 test.out && rm -f test.out && \
		for file in $$files; do \
		  test -z "`git diff --name-only $$file`" || continue; \
		  cmd="touch -d @`git log -1 --format='format:%ct' $$file \
			` $$file" && \
		  echo "$$cmd" && \
		  $$cmd || exit; \
		done

# The zics below ensure that each data file can stand on its own.
# We also do an all-files run to catch links to links.

public:		$(ENCHILADA) set-timestamps
		make maintainer-clean
		make "CFLAGS=$(GCC_DEBUG_FLAGS)"
		mkdir tzpublic
		for i in $(TDATA) ; do \
		  $(zic) -v -d tzpublic $$i 2>&1 || exit; \
		done
		$(zic) -v -d tzpublic $(TDATA)
		rm -fr tzpublic

# Check that the code works under various alternative
# implementations of time_t.
check_time_t_alternatives:
		zones=`$(AWK) '/^[^#]/ { print $$3 }' <time.tab` && \
		for type in $(TIME_T_ALTERNATIVES); do \
		  mkdir -p tzpublic/$$type && \
		  make clean_misc && \
		  make TOPDIR=`pwd`/tzpublic/$$type \
		    CFLAGS='$(CFLAGS) -Dtime_tz='"'$$type'" \
		    install && \
		  diff -qr tzpublic/int64_t/etc/zoneinfo tzpublic/$$type/etc/zoneinfo && \
		  case $$type in \
		  int32_t) range=-2147483648,2147483647;; \
		  uint32_t) range=0,4294967296;; \
		  int64_t) continue;; \
		  *u*) range=0,10000000000;; \
		  *) range=-10000000000,10000000000;; \
		  esac && \
		  echo checking $$type zones ... && \
		  tzpublic/int64_t/etc/zdump -V -t $$range $$zones \
		      >tzpublic/int64_t.out && \
		  tzpublic/$$type/etc/zdump -V -t $$range $$zones \
		      >tzpublic/$$type.out && \
		  diff -u tzpublic/int64_t.out tzpublic/$$type.out \
		    || exit; \
		done
		rm -fr tzpublic

tarballs:	tzcode$(VERSION).tar.gz tzdata$(VERSION).tar.gz

tzcode$(VERSION).tar.gz: $(COMMON) $(DOCS) $(SOURCES) $(MISC)
		for i in *.[1-8] ; do \
		  LC_ALL=C sh workman.sh $$i > $$i.txt && \
		  touch -r $$i $$i.txt || exit; \
		done
		$(AWK) -f checktab.awk $(PRIMARY_YDATA)
		LC_ALL=C && export LC_ALL && \
		tar $(TARFLAGS) -cf - $(DOCS) $(SOURCES) $(MISC) *.[1-8].txt | \
		  gzip $(GZIPFLAGS) > tzcode$(VERSION).tar.gz
		LC_ALL=C && export LC_ALL && \
		tar $(TARFLAGS) -cf - $(DATA) | \
		  gzip $(GZIPFLAGS) > tzdata$(VERSION).tar.gz

typecheck:
		make clean
		for i in "long long" unsigned double; \
		do \
			make CFLAGS="-DTYPECHECK -D_TIME_T \"-Dtime_t=$$i\"" ; \
			./zdump -v Europe/Rome ; \
			make clean ; \
		done

zonenames:	$(TDATA)
		@$(AWK) '/^Zone/ { print $$2 } /^Link/ { print $$3 }' $(TDATA)

CLEAN:		clean
			sccs clean

.KEEP_STATE:

.PHONY: ALL INSTALL all
.PHONY: check check_character_set check_public check_tables
.PHONY: check_time_t_alternatives check_web clean clean_misc
.PHONY: install maintainer-clean names posix_only posix_right
.PHONY: public right_only right_posix signatures tarballs typecheck
.PHONY: zonenames zones
