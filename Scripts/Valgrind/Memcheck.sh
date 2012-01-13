SUPPRESSIONS_FILE="`dirname $0`/`uname -s`.supp"
echo "SUPPRESSIONS_FILE=$SUPPRESSIONS_FILE"
valgrind --tool=memcheck --suppressions=${SUPPRESSIONS_FILE} --dsymutil=yes --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes 2>valgrind.out $*
