git submodule update --init --recursive
git submodule update --remote --merge

cd util
cmake .
make
cd ..
