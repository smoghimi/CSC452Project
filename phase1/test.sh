#!/bin/sh
# $0 is the script name, $1 id the first ARG, $2 is second...
make clean > temp.out
rm myTests/*
for VAR in {0..9}
do
	make test0"$VAR" > temp.out
	./test0"$VAR" | grep -v '^ ' | grep -v '^PID' > myTests/myTest0"$VAR".out
	cat testResults/test0"$VAR".txt | grep -v '^ ' | grep -v '^PID' > myTests/test0"$VAR".out
	diff myTests/myTest0"$VAR".out myTests/test0"$VAR".out
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "PASSED test0$VAR"
	else
		echo "FAILED test0$VAR"
	fi
done

for VAR in {10..36}
do
	make test"$VAR" > temp.out
	./test"$VAR" | grep -v '^ ' | grep -v '^PID' > myTests/myTest"$VAR".out
	cat testResults/test"$VAR".txt | grep -v '^ ' | grep -v '^PID' > myTests/test"$VAR".out
	diff myTests/myTest"$VAR".out myTests/test"$VAR".out
	ret=$?
	if [ $ret -eq 0 ]; then
		echo "PASSED test$VAR"
	else
		echo "--FAILED test$VAR--"
	fi
done

rm myTests/*