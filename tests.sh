#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

SIGN='Famine version 1.0 (c)oded by eparisot'

function check() {
	if strings $1 | grep "$SIGN" > /dev/null; then
		printf "${GREEN}[OK]${NC}\n"
	else
		printf "${RED}[FAIL]${NC}\n"
	fi
}

make re

printf "\n${RED}Famine simple tests: ${NC}\n"
./Famine

printf "\n${YELLOW}* infected hello infects fresh hello: ${NC}\n"
gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/hello
/tmp/test2/hello
check /tmp/test2/hello
printf "\n${YELLOW}* infected hello2 infects fresh hello2: ${NC}\n"
gcc ressources/sample2.c -o /tmp/test2/hello2
/tmp/test/hello2
/tmp/test2/hello2
check /tmp/test2/hello2
printf "\n${YELLOW}* infected ls infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/ls
/tmp/test2/ls
check /tmp/test2/ls
printf "\n${YELLOW}* infected date infects fresh date: ${NC}\n"
cp /bin/date /tmp/test2/date
/tmp/test/date
/tmp/test2/date
check /tmp/test2/date
printf "\n${YELLOW}* infected grep infects fresh grep: ${NC}\n"
cp /bin/grep /tmp/test2/grep
/tmp/test/grep
/tmp/test2/grep
check /tmp/test2/grep

printf "\n${YELLOW}* infected ls infects fresh hello: ${NC}\n"
gcc ressources/sample.c -o /tmp/test2/hello
/tmp/test/ls
/tmp/test2/hello
check /tmp/test2/hello
printf "\n${YELLOW}* infected hello infects fresh hello2: ${NC}\n"
gcc ressources/sample2.c -o /tmp/test2/hello2
/tmp/test/hello
/tmp/test2/hello2
check /tmp/test2/hello2
printf "\n${YELLOW}* infected hello infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/hello
/tmp/test2/ls
check /tmp/test2/ls
printf "\n${YELLOW}* infected hello2 infects fresh ls: ${NC}\n"
cp /bin/ls /tmp/test2/ls
/tmp/test/hello2
/tmp/test2/ls -la /tmp
check /tmp/test2/ls
printf "\n${YELLOW}* infected hello infects fresh date: ${NC}\n"
cp /bin/date /tmp/test2/date
/tmp/test/hello
/tmp/test2/date
check /tmp/test2/date
printf "\n${YELLOW}* infected hello infects fresh grep: ${NC}\n"
cp /bin/grep /tmp/test2/grep
/tmp/test/hello
cat ressources/sample.c | /tmp/test2/grep 'int'
check /tmp/test2/hello

echo ""

while true; do
    read -p "Run tests over all /bin/ ? (y/n): " yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer yes or no.";;
    esac
done

printf "\n${RED}Test all ELF64 from /bin/* : ${NC}\n"
for f in /bin/*
do 
	if file $f | grep -q 'ELF 64'; then
		cp $f /tmp/test/
	fi
done


printf "Running Famine..."
./Famine
printf "Done.\n"
for f in /tmp/test/*
do 
	if strings $f | grep "$SIGN" > /dev/null; then
		printf "${GREEN}.${NC}"
	else
		printf "${RED}.${NC}"
		#printf "\n${RED} Failed injection on $f ${NC}\n"
		#break
	fi
done

echo ""