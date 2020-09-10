git submodule update --init --recursive

cd util
cmake .
make
cd ..

cd libs/lit/html/
./build_html.sh
cd ../../../

cd libs/box2d/
emcmake cmake -DBOX2D_BUILD_DOCS=OFF -DBOX2D_BUILD_TESTBED=OFF -DBOX2D_BUILD_UNIT_TESTS=OFF .
emmake make
cd ../../

emcmake cmake .
emmake make
