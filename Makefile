# %W%

# If you want something other than Eastern United States time used on your
# system, change the line below (after finding the zone you want in the
# time zone files, or adding it to a time zone file).
# Alternately, if you discover you've got the wrong time zone, you can just
#	zic -l rightzone

LOCALTIME=	US/Eastern

# If you want something other than Eastern United States time as a template
# for handling POSIX-style time zone environment variables,
# change the line below (after finding the zone you want in the
# time zone files, or adding it to a time zone file).
# Alternately, if you discover you've got the wrong time zone, you can just
#	zic -p rightzone

POSIXRULES=	US/Eastern

# Use an absolute path name for TZDIR unless you're just testing the software.

TZDIR=		/etc/zoneinfo

# If you always want time values interpreted as "seconds since the epoch
# (not counting leap seconds)", use
# 	REDO=		posix_only
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

REDO=		posix_right

# You may want to change this define if you're just testing the software.
# Alternatively, you can put these functions in /lib/libc.a, removing
# the old "ctime.o".  This is the
# ideal solution if you are able.  Build libz.a, extract the files, and
# then add them to libc.a.

TZLIB=		/usr/lib/libz.a

# If you're running on a System V-style system and don't want lint grief,
# add
#	-DUSG
# to the end of the "CFLAGS=" line.
#
# If you're running on a system where "strchr" is known as "index",
# (for example, a 4.[012]BSD system), add
#	-Dstrchr=index
# to the end of the "CFLAGS=" line.
#
# If you want to use System V compatibility code, add
#	-DUSG_COMPAT
# to the end of the "CFLAGS=" line.
#
# If you want BSD compatibility code, add
#	-DBSD_COMPAT
# to the end of the "CFLAGS=" line.
#
# If your system has a "GMT offset" field in its "struct tm"s
# (or if you decide to add such a field in your system's "time.h" file),
# add the name to a define such as
#	-DTM_GMTOFF=tm_gmtoff
# or
#	-DTM_GMTOFF=_tm_gmtoff
# to the end of the "CFLAGS=" line.
#
# If your system has a "GMT offset" field in its "struct tm"s
# (or if you decide to add such a field in your system's "time.h" file),
# add the name to a define such as
#	-DTM_ZONE=tm_zone
# or
#	-DTM_ZONE=_tm_zone
# to the end of the "CFLAGS=" line.
#
# If you want code inspired by certain emerging standards, add
#	-DSTD_INSPIRED
# to the end of the "CFLAGS=" line.
#
# If you want Source Code Control System ID's left out of object modules, add
#	-DNOID
# to the end of the "CFLAGS=" line.
#
# If you'll never want to handle solar-time-based time zones, add
#	-DNOSOLAR
# to the end of the "CFLAGS=" line
# (and comment out the "SDATA=" line below).
#
# If you want to allocate state structures in localtime, add
#	-DALL_STATE
# to the end of the "CFLAGS=" line.
#
# If you want an "altzone" variable (a la System V Release 3.1), add
#	-DALTZONE
# to the end of the "CFLAGS=" line.
#
# If you want a "gtime" function (a la MACH), add
#	-DCMUCS
# to the end of the "CFLAGS=" line

CFLAGS=

# LINTFLAGS is set for 4.[123]BSD systems.
# If you're using System V, you'll want to comment out the "LINTFLAGS=" line.

LINT=		lint
LINTFLAGS=	-hbaxc

SHAR=		shar

################################################################################

CC=		cc -DTZDIR=\"$(TZDIR)\"

TZCSRCS=	zic.c localtime.c asctime.c \
		scheck.c ialloc.c emkdir.c getopt.c link.c
TZCOBJS=	zic.o localtime.o asctime.o \
		scheck.o ialloc.o emkdir.o getopt.o link.o
TZDSRCS=	zdump.c localtime.c asctime.c \
		ialloc.c getopt.c link.c
TZDOBJS=	zdump.o localtime.o asctime.o \
		ialloc.o getopt.o link.o
LIBSRCS=	localtime.c asctime.c ctime.c dysize.c timemk.c difftime.c
LIBOBJS=	localtime.o asctime.o ctime.o dysize.o timemk.o difftime.o
HEADERS=	tzfile.h nonstd.h stdio.h stdlib.h time.h
NONLIBSRCS=	zic.c zdump.c \
		scheck.c ialloc.c emkdir.c getopt.c link.c
SOURCES=	$(HEADERS) $(LIBSRCS) $(NONLIBSRCS)
DOCS=		Patchlevel.h \
		README Theory \
		newctime.3 tzfile.5 zic.8 zdump.8 \
		Makefile
YDATA=		africa antarctica asia australasia \
		europe northamerica southamerica pacificnew
NDATA=		systemv
SDATA=		solar87 solar88 solar89
TDATA=		$(YDATA) $(NDATA) $(SDATA)
DATA=		$(YDATA) $(NDATA) $(SDATA) leapseconds
USNO=		usno1988 usno1989
ENCHILADA=	$(DOCS) $(SOURCES) $(DATA) $(USNO)

all:		REDID_BINARIES zdump $(TZLIB)

REDID_BINARIES:	zic $(DATA) $(REDO)
		./zic -d $(TZDIR) -l $(LOCALTIME) -p $(POSIXRULES)
		touch $@

posix_only:
		./zic -d $(TZDIR) -L /dev/null $(TDATA)

right_only:
		./zic -d $(TZDIR) -L leapseconds $(TDATA)

other_two:
		./zic -d $(TZDIR)/posix -L /dev/null $(TDATA)
		./zic -d $(TZDIR)/right -L leapseconds $(TDATA)

posix_right:	posix_only other_two

right_posix:	right_only other_two

zdump:		$(TZDOBJS)
		$(CC) $(CFLAGS) $(LFLAGS) $(TZDOBJS) -o $@

$(TZLIB):	$(LIBOBJS)
		ar ru $@ $(LIBOBJS)
		-ranlib $@

zic:		$(TZCOBJS)
		$(CC) $(CFLAGS) $(LFLAGS) $(TZCOBJS) -o $@

SHARS:		SHAR1.Z.uue SHAR2.Z.uue SHAR3.Z.uue \
		SHAR4.Z.uue SHAR5.Z.uue SHAR6.Z.uue

SHAR1.Z.uue:	$(DOCS)
		$(SHAR) $(DOCS) | compress | uuencode SHAR1.Z > $@

SHAR2.Z.uue:	$(HEADERS) $(LIBSRCS)
		$(SHAR) $(HEADERS) $(LIBSRCS) | compress | uuencode SHAR2.Z > $@

SHAR3.Z.uue:	$(NONLIBSRCS)
		$(SHAR) $(NONLIBSRCS) | compress | uuencode SHAR3.Z > $@

SHAR4.Z.uue:	$(YDATA) $(NDATA)
		$(SHAR) $(YDATA) $(NDATA) | compress | uuencode SHAR4.Z > $@

SHAR5.Z.uue:	$(SDATA)
		$(SHAR) $(SDATA) | compress | uuencode SHAR5.Z > $@

SHAR6.Z.uue:	$(USNO)
		$(SHAR) $(USNO) | compress | uuencode SHAR6.Z > $@

sure:		$(SOURCES)
		$(LINT) $(LINTFLAGS) $(CFLAGS) -DTZDIR=\"$(TZDIR)\" $(TZCSRCS)
		$(LINT) $(LINTFLAGS) $(CFLAGS) -DTZDIR=\"$(TZDIR)\" $(TZDSRCS)
		$(LINT) $(LINTFLAGS) $(CFLAGS) -DTZDIR=\"$(TZDIR)\" $(LIBSRCS)

SURE:		sure $(ENCHILADA)
		make sure LINT=/usr/5bin/lint LINTFLAGS=""
		make sure LINT=gcc LINTFLAGS="-c -O -ansi -pedantic -Wall"
		rm -f zic zdump *.o
		spell $(ENCHILADA)

clean:
		rm -f core *.o *.out REDID_BINARIES zdump zic \
		SHAR* tz.shar.Z.uue ,*

CLEAN:		clean
		sccs clean

names:
		@echo $(ENCHILADA)

asctime.o:	nonstd.h stdio.h time.h tzfile.h
ctime.o:	nonstd.h time.h
dysize.o:	tzfile.h
emkdir.o:	nonstd.h stdlib.h
ialloc.o:	nonstd.h stdlib.h
link.o:		nonstd.h stdio.h
localtime.o:	nonstd.h stdio.h stdlib.h time.h tzfile.h
scheck.o:	nonstd.h stdio.h stdlib.h
zdump.o:	nonstd.h stdio.h stdlib.h time.h tzfile.h
zic.o:		nonstd.h stdio.h stdlib.h time.h tzfile.h

.KEEP_STATE:
