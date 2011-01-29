:PHONY Darwin Xcode Linux Cygwin

UNAME=$(shell uname)
all: ${UNAME}

Cygwin:
	echo "Making Cygwin"
	Scripts/Make/MakeCygwin.sh	

Linux:
	echo "Making Linux"
	Scripts/Make/MakeLinux.sh

Darwin:
	echo "Making Darwin"
	Scripts/Make/MakeDarwin.sh

Xcode:
	echo "Making Xcode"
	Scripts/Make/MakeXcode.sh

