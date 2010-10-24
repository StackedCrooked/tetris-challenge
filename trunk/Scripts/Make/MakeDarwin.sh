rm -rf Build/
mkdir Build
cd Build
export CMAKE_OSX_ARCHITECTURES=i386
cmake ..
cd Build
make VERBOSE=1 -j6
echo ""
cd .. 
echo "Created executable: `find Build -name test`"
