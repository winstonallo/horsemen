#!/usr/bin/env bash

make test && ./scripts/copy_hosts.sh && ./Pestilence && strings /tmp/test/$1 | grep Pes && valgrind /tmp/test/$1 --help && echo "All tests passed!" && readelf -S Pestilence | grep -A 1 .text
