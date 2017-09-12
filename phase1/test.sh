#!/bin/sh
# $0 is the script name, $1 id the first ARG, $2 is second...
make test"$1"
./test"$1" | grep -v '^ ' | grep -v '^PID' > myTests/myTest"$1".out
cat testResults/test"$1".txt | grep -v '^ ' | grep -v '^PID' > myTests/test"$1".out
diff myTests/myTest"$1".out myTests/test"$1".out