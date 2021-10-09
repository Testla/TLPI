set -e

"$1" /dev/null 2>&1 | grep -q "is not a"

temp_dir=$(mktemp -d /tmp/tmp.XXXXXXXXXX)
temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file' && rmdir '$temp_dir'" EXIT

chmod 0 "$temp_dir"
chmod 0 "$temp_file"
"$1" "$temp_dir" "$temp_file"
[ $(stat -c %a "$temp_dir") = 555 ]
[ $(stat -c %a "$temp_file") = 444 ]
chmod g+x "$temp_file"
"$1" "$temp_file"
[ $(stat -c %a "$temp_file") = 555 ]

rm -f -- "$temp_file" && rmdir "$temp_dir"
trap - EXIT
