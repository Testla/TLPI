#!/bin/bash

set -e

temp_file=$(mktemp /tmp/tmp.XXXXXXXXXX)
trap "rm -f -- '$temp_file'" EXIT

"$1" = "$temp_file"

"$1" +Ads "$temp_file"
# Use long names which are easier to check.
attrs=$(lsattr -l "$temp_file")
[[ $attrs =~ "No_Atime" ]]
[[ $attrs =~ "No_Dump" ]]
[[ $attrs =~ "Secure_Deletion" ]]
! [[ $attrs =~ "Synchronous_Updates" ]]

"$1" -dsS "$temp_file"
attrs=$(lsattr -l "$temp_file")
[[ $attrs =~ "No_Atime" ]]
! [[ $attrs =~ "No_Dump" ]]
! [[ $attrs =~ "Secure_Deletion" ]]
! [[ $attrs =~ "Synchronous_Updates" ]]

"$1" = "$temp_file"
attrs=$(lsattr -l "$temp_file")
! [[ $attrs =~ "No_Atime" ]]

"$1" +a "$temp_file" 2>&1 | grep -q EPERM -

sudo "$1" +a "$temp_file"
attrs=$(lsattr -l "$temp_file")
[[ $attrs =~ "Append_Only" ]]
sudo "$1" -a "$temp_file"

rm -f -- "$temp_file"
trap - EXIT
