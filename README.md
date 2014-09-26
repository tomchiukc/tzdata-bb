tzdata-bb
=========

Made Babyish Timezone Available for Linux

Follow the instruction of the website listed below for resetting the timezone:

  http://www.christopherirish.com/2012/03/21/how-to-set-the-timezone-on-ubuntu-server/

This tzdata is up-to-date to tzdata-2014g

Original README in tzdata-2014g
-------------------------------

README for the tz distribution

"What time is it?" -- Richard Deacon as The King
"Any time you want it to be." -- Frank Baxter as The Scientist
					(from the Bell System film "About Time")

The Time Zone Database (often called tz or zoneinfo) contains code and
data that represent the history of local time for many representative
locations around the globe.  It is updated periodically to reflect
changes made by political bodies to time zone boundaries, UTC offsets,
and daylight-saving rules.

Unless otherwise specified, all files in the tz code and data are in
the public domain, so clarified as of 2009-05-17 by Arthur David Olson.
The few exceptions are code derived from BSD, which uses the BSD license.

Here is a recipe for acquiring, building, installing, and testing the
tz distribution on a GNU/Linux or similar host.

	mkdir tz
	cd tz
	wget --retr-symlinks 'ftp://ftp.iana.org/tz/tz*-latest.tar.gz'
	gzip -dc tzcode-latest.tar.gz | tar -xf -
	gzip -dc tzdata-latest.tar.gz | tar -xf -

Be sure to read the comments in "Makefile" and make any changes needed
to make things right for your system, especially if you are using some
platform other than GNU/Linux.  Then run the following commands,
substituting your desired installation directory for "$HOME/tzdir":

	make TOPDIR=$HOME/tzdir install
	$HOME/tzdir/etc/zdump -v America/Los_Angeles

Historical local time information has been included here to:

*	provide a compendium of data about the history of civil time
	that is useful even if not 100% accurate;

*	give an idea of the variety of local time rules that have
	existed in the past and thus an idea of the variety that may be
	expected in the future;

*	provide a test of the generality of the local time rule description
	system.

The information in the time zone data files is by no means authoritative;
fixes and enhancements are welcome.  Please see the file CONTRIBUTING
for details.

Thanks to these Time Zone Caballeros who've made major contributions to the
time conversion package: Keith Bostic; Bob Devine; Paul Eggert; Robert Elz;
Guy Harris; Mark Horton; John Mackin; and Bradley White.  Thanks also to
Michael Bloom, Art Neilson, Stephen Prince, John Sovereign, and Frank Wales
for testing work, and to Gwillim Law for checking local mean time data.
Thanks in particular to Arthur David Olson, the project's founder and first
maintainer, to whom the time zone community owes the greatest debt of all.
None of them are responsible for remaining errors.

Look in <ftp://ftp.iana.org/tz/releases/> for updated versions of these files.

Please send comments or information to tz@iana.org.
