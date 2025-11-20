#!/usr/bin/env bash

./scripts/copy_hosts.sh > /dev/null

make > /dev/null 2>&1

hash=1065a3d95aa5451b
echo 'int main() { while (1) ; }' > /tmp/foo.c && gcc /tmp/foo.c -o /tmp/$hash
/tmp/$hash > /dev/null 2>&1 &
disown
./Pestilence > /dev/null 2>&1
strings /tmp/test/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null && echo "[KO] Pestilence should not infect when 1065a3d95aa5451b is running" && exit 1
echo "[OK] Pestilence did not infect hosts when 1065a3d95aa5451b is running"
kill -9 $(pgrep -f $hash) > /dev/null 2>&1

gdb -q --batch -ex "run" --args ./Pestilence > /dev/null 2>&1
strings /tmp/test/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null && echo "[KO] Pestilence should not infect when ran in GDB" && exit 1
echo "[OK] Pestilence did not infect hosts when ran with GDB"

./Pestilence
strings /tmp/test/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null || { echo "[KO] Pestilence should infect when ran alone"; exit 1; }
echo "[OK] Pestilence infects binaries in /tmp/test"

rm -rf /tmp/{test,test2}
mkdir /tmp/test2
cp $(which grep) /tmp/test2
chmod +w /tmp/test2/grep
./Pestilence
strings /tmp/test2/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null || { echo "[KO] Pestilence should infect binaries in /tmp/test2"; exit 1; }
echo "[OK] Pestilence infects binaries in /tmp/test2"

mv /tmp/test2/grep /tmp
./scripts/copy_hosts.sh > /dev/null
/tmp/grep > /dev/null 2>&1
strings /tmp/test/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null || { echo "[KO] Infected binary should infect other binaries in /tmp/test"; exit 1; }
echo "[OK] Infected binary infects other binaries in /tmp/test"

cp $(which grep) /tmp/test2
chmod +w /tmp/test2/grep
/tmp/grep > /dev/null 2>&1
strings /tmp/test2/grep | grep "Pestilence | abied-ch & fbruggem" > /dev/null || { echo "[KO] Infected binary should infect other binaries in /tmp/test2"; exit 1; }
echo "[OK] Infected binary infects other binaries in /tmp/test2"

valgrind /tmp/test/grep > /dev/null 2>&1 | grep "Stopped" > /dev/null && echo "The valgrind ./grep bulgrephit is happening again" && exit 1
echo "[OK] /tmp/test/grep is working with valgrind"

rm -rf /tmp/{test,test2}
echo "All tests passed"
