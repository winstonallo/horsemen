#!/bin/bash

make > /dev/null
echo "int main() { return 0; }" > /tmp/test.c
cc /tmp/test.c -o /tmp/test/sample
./Famine > /dev/null
strings /tmp/test/sample | grep "abied-ch" > /dev/null
