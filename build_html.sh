emcmake cmake .
emmake make

emcc -o3 dist/tsab.a dist/tsab.bc -I include/ -s ALLOW_MEMORY_GROWTH=1 -s RESERVED_FUNCTION_POINTERS=1 -s NO_EXIT_RUNTIME=1 --shell-file html/shell.html -o dist/index.html -s WASM=1 -s EXPORTED_FUNCTIONS='[]' -s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall", "cwrap"]'