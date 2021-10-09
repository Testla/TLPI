#!/bin/bash

set -e

"$1" -f /etc/shadow
"$1" -r /etc/shadow 2>&1 | grep -q EACCES -
"$1" -fr /etc/shadow 2>&1 | grep -q EACCES -
"$1" -frx /etc
"$1" -fwrx /etc 2>&1 | grep -q EACCES -

sudo chown root "$1"
sudo chmod u+s "$1"

"$1" -frw /etc/shadow
"$1" -x /etc/shadow 2>&1 | grep -q EACCES -
"$1" -fwrx /etc

sudo chown $(whoami) "$1"
sudo chmod u-s "$1"
