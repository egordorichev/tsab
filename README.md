# tsab

**tsab** is a game development framework for [lit](https://github.com/egordorichev/lit). It's heavily inspired by Love2d, but it also supports HTML5. Android branch is under development.

## Installation

On linux (using ubuntu for this example) you will need to install these packages:

```bash
sudo apt install cmake libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev freeglut3-dev libglew-dev
```

After that, build the submodules:

```
./build_deps.sh
```

And the usual cmake stuff:

```
cmake .
make
sudo make install
```