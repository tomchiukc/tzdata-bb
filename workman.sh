#! /bin/sh

# This file is in the public domain, so clarified as of
# 2009-05-17 by Arthur David Olson.

# Tell groff not to emit SGR escape sequences (ANSI color escapes).
GROFF_NO_SGR=1
export GROFF_NO_SGR

echo ".am TH
.hy 0
.na
..
.rm }H
.rm }F" | nroff -man - ${1+"$@"} | perl -ne '
	binmode STDIN, '\'':encoding(utf8)'\'';
	binmode STDOUT, '\'':encoding(utf8)'\'';
	chomp;
<<<<<<< HEAD
	s/.//g;
	s/[ 	]*$//;
=======
	s/.\010//g;
	s/\s*$//;
>>>>>>> grandpa/master
	if (/^$/) {
		$sawblank = 1;
		next;
	} else {
<<<<<<< HEAD
		if ($sawblank) {
=======
		if ($sawblank && $didprint) {
>>>>>>> grandpa/master
			print "\n";
			$sawblank = 0;
		}
		print "$_\n";
<<<<<<< HEAD
	}
' | case $ttyval in
	0)	more ;;
	*)	cat ;;
esac
=======
		$didprint = 1;
	}
'
>>>>>>> grandpa/master
