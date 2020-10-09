git submodule update --init --recursive
git submodule update --remote --merge

# No idea why this has to be done but it doesn't pull source files sometimes
cd libs/box2d/
git checkout master
cd ../SDL_gpu
git checkout master
cd ../../

cd util
cmake .
make
cd ..