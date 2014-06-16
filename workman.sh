#! /bin/sh

tty -s
ttyval=$?

echo ".am TH
.hy 0
.na
..
.rm }H
.rm }F" | nroff -man - ${1+"$@"} | perl -ne '
	binmode STDIN, '\'':encoding(utf8)'\'';
	binmode STDOUT, '\'':encoding(utf8)'\'';
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
