#!/usr/bin/env bash

./scripts/copy_hosts.sh > /dev/null

make > /dev/null 2>&1

hash=1065a3d95aa5451b
echo 'int main() { while (1) ; }' > /tmp/foo.c && gcc /tmp/foo.c -o /tmp/$hash
/tmp/$hash > /dev/null 2>&1 &
./Pestilence > /dev/null 2>&1
strings /tmp/test/ls | grep Pestilence > /dev/null && echo "Pestilence should not infect when 1065a3d95aa5451b is running" && exit 1
echo "Pestilence did not infect hosts when 1065a3d95aa5451b is running"
kill -9 $(pgrep -f $hash) > /dev/null 2>&1

gdb -q --batch -ex "run" --args ./Pestilence > /dev/null 2>&1
strings /tmp/test/ls | grep Pestilence > /dev/null && echo "Pestilence should not infect when ran in GDB" && exit 1
echo "Pestilence did not infect hosts when ran with GDB"

./Pestilence
strings /tmp/test/ls | grep Pestilence > /dev/null || { echo "Pestilence should infect when ran alone"; exit 1; }
echo "Pestilence infected hosts when ran alone"

echo "Success"
