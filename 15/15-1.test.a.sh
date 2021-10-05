set -e

temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file'" EXIT
chmod 066 $temp_file

! cat $temp_file
! echo 123 > $temp_file

rm -f -- "$temp_file"
trap - EXIT
