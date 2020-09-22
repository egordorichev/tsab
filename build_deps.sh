git submodule update --init --recursive

cd util
cmake .
make
cd ..

cmake .
cd libs/box2d/
./build.sh
cd ../../