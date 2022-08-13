#!/bin/sh

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

make re

printf "\n${RED}Famine simple tests: ${NC}\n"
./Famine

printf "\n${YELLOW}* infected hello infects fresh hello: ${NC}\n"
gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/hello
/tmp/test2/hello
printf "\n${YELLOW}* infected hello2 infects fresh hello2: ${NC}\n"
gcc ressources/sample2.c -o /tmp/test2/hello2
/tmp/test/hello2
/tmp/test2/hello2
printf "\n${YELLOW}* infected ls infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/ls
/tmp/test2/ls
printf "\n${YELLOW}* infected date infects fresh date: ${NC}\n"
cp /bin/date /tmp/test2/date
/tmp/test/date
/tmp/test2/date

printf "\n${YELLOW}* infected grep infects fresh grep: ${NC}\n"
cp /bin/grep /tmp/test2/grep
/tmp/test/grep
/tmp/test2/grep


printf "\n${YELLOW}* infected ls infects fresh hello: ${NC}\n"
gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/ls
/tmp/test2/hello
printf "\n${YELLOW}* infected hello infects fresh hello2: ${NC}\n"
gcc ressources/sample2.c -o /tmp/test2/hello2
/tmp/test/hello
/tmp/test2/hello2
printf "\n${YELLOW}* infected hello infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/hello
/tmp/test2/ls
printf "\n${YELLOW}* infected hello2 infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/hello2
/tmp/test2/ls
printf "\n${YELLOW}* infected hello infects fresh date: ${NC}\n"
cp /bin/date /tmp/test2/date
/tmp/test/hello
/tmp/test2/date
printf "\n${YELLOW}* infected hello infects fresh grep: ${NC}\n"
cp /bin/grep /tmp/test2/grep
/tmp/test/hello
cat ressources/sample.c | /tmp/test2/grep 'int'