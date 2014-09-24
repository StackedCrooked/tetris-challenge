all:
	([ -d Build ] || mkdir Build ; ./configure ; cd Build ; make -j11)
