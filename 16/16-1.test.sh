set -e

temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file'" EXIT

value=Hello,World!

"$1" "$temp_file" user.x $value
# You may need to install the attr package first.
getfattr -d "$temp_file" | grep -q $value -

"$1" "$temp_file" trusted.x $value 2>&1 | grep -q EPERM -

sudo "$1" "$temp_file" trusted.y $value
sudo getfattr -n trusted.y "$temp_file" | grep -q $value -

rm -f -- "$temp_file"
trap - EXIT
