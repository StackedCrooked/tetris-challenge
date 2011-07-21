mkdir -p Projects
rm -rf Projects/Xcode
mkdir Projects/Xcode
cd Projects/Xcode
cmake -G Xcode ../..
cd ../..
echo
echo "Created Xcode projects:"
find Projects/Xcode -name "*.xcodeproj" | grep -v CMakeTmp
