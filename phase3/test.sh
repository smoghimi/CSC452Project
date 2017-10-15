#!/bin/sh
# $0 is the script name, $1 id the first ARG, $2 is second...
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'
#make clean > temp.out
rm myTests/* > temp.out
for VAR in {0..4}
do
	make test0"$VAR" > temp.out
	./test0"$VAR" | grep -v '^ ' | grep -v '^PID' > myTests/myTest0"$VAR".out
	cat testResults/test0"$VAR".txt | grep -v '^ ' | grep -v '^PID' > myTests/test0"$VAR".out
	diff myTests/myTest0"$VAR".out myTests/test0"$VAR".out > tmp.out
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "${GREEN}PASSED test0$VAR${NC}"
	else
		echo "${RED}FAILED test0$VAR${NC}"
	fi
done

: <<'END'
for VAR in {10..12}
do
	make test"$VAR" > temp.out
	./test"$VAR" | grep -v '^ ' | grep -v '^PID' > myTests/myTest"$VAR".out
	cat testResults/test"$VAR".txt | grep -v '^ ' | grep -v '^PID' > myTests/test"$VAR".out
	diff myTests/myTest"$VAR".out myTests/test"$VAR".out
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "${GREEN}PASSED test$VAR${NC}"
	else
		echo "${RED}--FAILED test$VAR--${NC}"
	fi
done

for VAR in {14..26}
do
	make test"$VAR" > temp.out
	./test"$VAR" | grep -v '^ ' | grep -v '^PID' > myTests/myTest"$VAR".out
	cat testResults/test"$VAR".txt | grep -v '^ ' | grep -v '^PID' > myTests/test"$VAR".out
	diff myTests/myTest"$VAR".out myTests/test"$VAR".out
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "${GREEN}PASSED test$VAR${NC}"
	else
		echo "${RED}--FAILED test$VAR--${NC}"
	fi
done
END
#make clean > temp.out
rm temp.out tmp.out
rm myTests/*