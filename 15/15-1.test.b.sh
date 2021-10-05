set -e

temp_dir=$(mktemp -d /tmp/tmp.XXXXXXXXXX)
temp_file=$(mktemp $temp_dir/tmp.XXXXXXXXXX)
trap "chmod u+x '$temp_dir' && rm -f -- '$temp_file' && rmdir '$temp_dir'" EXIT

echo 123 > $temp_file
cat $temp_file > /dev/null

chmod u-x $temp_dir
ls $temp_dir

! cat $temp_file
! echo 123 > $temp_file

chmod u+x "$temp_dir" && rm -f -- "$temp_file" && rmdir "$temp_dir"
trap - EXIT
