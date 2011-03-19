rm -rf Build/
mkdir -p Build/CMake
cd Build/CMake
cmake ../..
make VERBOSE=1 -j6
echo ""
