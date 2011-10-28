valgrind --tool=memcheck --dsymutil=yes --leak-check=yes --show-reachable=yes --num-callers=20 --track-fds=yes $1
