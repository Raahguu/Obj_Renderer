# Obj_Renderer
A C++ program that compiles into a 3D renderer, eventually ideally it will be able to read .obj files and display them

# installation instructions
If you downloaded the executable it should run on any system that suppports X11 (Linux)

If you want to compile then you will need to first download `libx11-dev`
```bash
$ sudo apt install libx11-dev
```

And then you will compile it:
```bash
cmake -S . -B build
cmake --build build -j
```
