set -e

temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file'" EXIT

first=$(stat "$temp_file")
sleep 1
second=$(stat "$temp_file")
[ "$first" = "$second" ] || exit 1

rm -f -- "$temp_file"
trap - EXIT
