[ -d "Build" ] || {
    ./rebuild
    exit
}


(cd Build && make -j11)
