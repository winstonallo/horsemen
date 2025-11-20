#!/usr/bin/env bash

mkdir -p ~/clamav/db

cat << EOF > /$HOME/clamav/freshclam.conf
DatabaseDirectory /home/winstonallo/clamav/db
UpdateLogFile /home/winstonallo/clamav/freshclam.log
Checks 1
DatabaseMirror database.clamav.net
Bytecode yes
EOF

freshclam --config-file=/$HOME/clamav/freshclam.conf


clamscan --database=/home/winstonallo/clamav/db -r --detect-pua=yes --heuristic-scan-precedence=yes /tmp/test/
