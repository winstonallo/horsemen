#!/usr/bin/env bash

./scripts/copy_hosts.sh > /dev/null

make > /dev/null 2>&1

valgrind ./Pestilence > /dev/null 2>&1
strings /tmp/test/ls | grep Pestilence > /dev/null && echo "Pestilence should not infect when ran with valgrind" && exit 1
echo "Pestilence did not infect hosts when ran with valgrind"

gdb -q --batch -ex "run" --args ./Pestilence > /dev/null 2>&1
strings /tmp/test/ls | grep Pestilence > /dev/null && echo "Pestilence should not infect when ran in GDB" && exit 1
echo "Pestilence did not infect hosts when ran with GDB"

./Pestilence
strings /tmp/test/ls | grep Pestilence > /dev/null || { echo "Pestilence should infect when ran alone"; exit 1; }
echo "Pestilence infected hosts when ran alone"

echo "Success"
