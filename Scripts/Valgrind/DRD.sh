valgrind --tool=drd --check-stack-var=yes --first-race-only=yes --free-is-write=yes 2>valgrind.out $1
