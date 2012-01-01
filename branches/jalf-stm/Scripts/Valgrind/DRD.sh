valgrind --tool=drd --check-stack-var=yes --segment-merging=no --first-race-only=no --read-var-info=yes --free-is-write=yes $* 2>valgrind.out
