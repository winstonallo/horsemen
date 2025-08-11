#!/bin/bash

make > /dev/null
mkdir -p /tmp/{test,test2}
echo "int main() { return 0; }" > /tmp/test.c
cc /tmp/test.c -o /tmp/test/sample
./Famine > /dev/null 2>&1
strings /tmp/test/sample | grep "abied-ch" > /dev/null
