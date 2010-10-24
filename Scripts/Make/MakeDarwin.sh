rm -rf Build/
mkdir -p Build/CMake
cd Build/CMake
export CMAKE_OSX_ARCHITECTURES=i386
cmake ../..
make VERBOSE=1 -j6
echo ""
echo "Created executable: `find ../../Build -name test`"
