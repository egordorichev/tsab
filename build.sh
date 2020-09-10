cd util
cmake .
make
cd ..

cmake .
cd libs/box2d/
./build.sh
cd ../../
make
