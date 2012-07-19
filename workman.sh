#! /bin/sh

# <pre>
# This file is in the public domain, so clarified as of
# 2009-05-17 by Arthur David Olson.

tty -s
ttyval=$?

case $# in
	0)	nroff -man ;;
	1)	if [ -f $1 ]
		then
			( echo .hy 0; echo .na ) | nroff -man - "$1"
		else
			man "$1"
		fi ;;
	*)	man ${1+"$@"} ;;
esac | perl -ne '
	if (($. % 66) <= 7) {
		next;
	}
	if (($. % 66) > (66 - 7)) {
		next;
	}
	chomp;
	s/.//g;
	s/[ 	]*$//;
	if (/^$/) {
		$sawblank = 1;
		next;
	} else {
		if ($sawblank) {
			print "\n";
			$sawblank = 0;
		}
		print "$_\n";
	}
' | case $ttyval in
	0)	more ;;
	*)	cat ;;
esac
