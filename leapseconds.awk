# Generate the 'leapseconds' file from 'leap-seconds.list'.

# This file is in the public domain.

BEGIN {
}

/^ *$/ { next }
/^#/ { next }

{
    NTP_timestamp = $1
    TAI_minus_UTC = $2
    hash_mark = $3
    one = $4
    month = $5
    year = $6
    if (old_TAI_minus_UTC) {
	if (old_TAI_minus_UTC < TAI_minus_UTC) {
	    sign = "23:59:60\t+"
	} else {
	    sign = "23:59:59\t-"
	}
	if (month == "Jan") {
	    year--;
	    month = "Dec";
	    day = 31
	} else if (month == "Jul") {
	    month = "Jun";
	    day = 30
	}
	printf "Leap\t%s\t%s\t%s\t%s\tS\n", year, month, day, sign
    }
    old_TAI_minus_UTC = TAI_minus_UTC
}
