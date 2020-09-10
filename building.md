Requires:

* SDL2
* SDL2 TTF
* SDL2 Mixer
* SDL2 Image
* Box2D (https://github.com/erincatto/Box2D)

# Building box2d

emcmake cmake -DBOX2D_BUILD_DOCS=OFF -DBOX2D_BUILD_EXAMPLES=OFF -DBOX2D_BUILD_TESTBED=OFF -DBOX2D_BUILD_UNIT_TESTS=OFF . 
emmake make

# Building lit

cd html
./build_html.sh