git submodule update --init --recursive
git submodule update --remote --merge

cd util
cmake .
make
cd ..

cmake .
cd libs/box2d/

# contents of ./build.sh + some flags
rm -rf build
mkdir build
cd build
cmake -DBOX2D_BUILD_DOCS=OFF -DBOX2D_BUILD_UNIT_TESTS=OFF -DBOX2D_BUILD_TESTBED=OFF ..
cmake --build .
# end contents

cd ../../