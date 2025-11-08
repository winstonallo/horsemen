#!/usr/bin/env bash

./scripts/copy_hosts.sh > /dev/null

make > /dev/null 2>&1

valgrind ./Famine > /dev/null 2>&1
strings /tmp/test/ls | grep Famine > /dev/null && echo "Famine should not infect when ran with valgrind" && exit 1
echo "Famine did not infect hosts when ran with valgrind"

gdb -q --batch -ex "run" --args ./Famine > /dev/null 2>&1
strings /tmp/test/ls | grep Famine > /dev/null && echo "Famine should not infect when ran in GDB" && exit 1
echo "Famine did not infect hosts when ran with GDB"

./Famine
strings /tmp/test/ls | grep Famine > /dev/null || { echo "Famine should infect when ran alone"; exit 1; }
echo "Famine infected hosts when ran alone"

echo "Success"
