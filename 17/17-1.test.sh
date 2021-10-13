#!/bin/bash
set -e

temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file'" EXIT

chmod 760 "$temp_file"
# Though root is privileged, we use it to avoid finding another user and group.
setfacl -m u:root:rx,g:root:wx,m::rw,o::- "$temp_file"

[ $("$1" u $(whoami) "$temp_file") = $(echo "rwx") ]
result=$("$1" u root "$temp_file")
expect=$(echo -e "r-x\nr--")
[ "$result" = "$expect" ]
result=$("$1" g $(groups | cut -d " " -f 1) "$temp_file")
expect=$(echo -e "rw-\nrw-")
[ "$result" = "$expect" ]
result=$("$1" g root "$temp_file")
expect=$(echo -e "-wx\n-w-")
[ "$result" = "$expect" ]

sudo chown root "$temp_file"
[ $("$1" u $(whoami) "$temp_file") = $(echo ---) ]
sudo chown $(whoami) "$temp_file"

rm -f -- "$temp_file"
trap - EXIT
