DIRS="Futile Tetris QtTetris"
for dir in $DIRS ; do
    cppcheck --enable=style --template gcc $dir 1>/dev/null
done
