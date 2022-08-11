#!/bin/sh

make re
./Famine

gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/hello
/tmp/test2/hello

gcc ressources/sample2.c -o /tmp/test2/hello2
/tmp/test/hello
/tmp/test2/hello2

cp /bin/ls /tmp/test2/ls
/tmp/test/ls
/tmp/test2/ls

gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/ls
/tmp/test2/hello

cp /bin/ls /tmp/test2/ls
/tmp/test/hello
/tmp/test2/ls

cp /bin/ls /tmp/test2/ls
/tmp/test/hello2
/tmp/test2/ls
