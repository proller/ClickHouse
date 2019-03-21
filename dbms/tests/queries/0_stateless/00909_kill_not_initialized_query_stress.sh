#!/usr/bin/env bash

set -e

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. $CURDIR/../shell_config.sh

for n in {1..10}; do
     $CURDIR/00909_kill_not_initialized_query.sh &
     #sleep 0.1
done

for n in {1..10}; do
     sleep 0.1
     $CURDIR/00909_kill_not_initialized_query.sh &
done

for n in {1..10}; do
     sleep 0.9
     $CURDIR/00909_kill_not_initialized_query.sh &
done

wait
