#!/bin/bash

set -e

# Usage: $0 executable file_to_create

$1

bash <(echo "sleep 10") &
export parent_pid=$!
# Wait for sleep child process to be created.
sleep 1
export child_pid=$(pgrep -P $parent_pid sleep)
echo $0 parent_pid $parent_pid child_pid $child_pid
$1 $child_pid 2 &
sleep 1 && kill $parent_pid && wait $!
kill $child_pid
touch $2
