# CSci 5607

## Assignment 0
This assignment was programmed in C and compiled on Windows using gcc via MinGW64.

### Provided files
The `turnin.tar` for this assignment should contain:
```
Makefile
Doxyfile
README.md
test.ppm
test.txt
src/main.c
src/ppm.h
src/ppm.c
```

### Building the code
There are no dependencies on external libraries. This project is expected to be compiled with `gcc` using the given `Makefile`. To build the project, run `make all`. This will create `main.exe` which is the program to run.

### Using `main.exe`
To use `main.exe`, run `main filename.txt` on the terminal where `filename.txt` refers to an input file of the format described in the handout. More specifically, input files must be of the following format:
```
imsize width height
```
In this format, `width` and `height` are positive integer values.

`main.exe` is compiled in debug mode by default. Any errors will be reported on the terminal. A successful run will result in `filename.ppm` being created. The result image will contain various 32*32 pixel gradient squares. The squares will increase in blueness as the sum of the x and y coordinates increases. The squares increase in redness as x increases and increase in greenness as y increases. This behavior is modular and defined mathematically in the `main` function. `test.ppm` was generated using `test.txt` and is an example of these images.

### Code
The code is documented using Doxygen-compatible commenting. To create the documentation webpage, run `make doc` and launch `doc/html/index.html` in a web browser. You must have `doxygen` installed.

Doxygen-style ocumentation is found in header files. The implementation files also contain comments, but not Doxygen comments.

In these files, I provide a suite of functions that deal with PPM structs. Correct usage of these functions can be found by reading the aforementioned documentation. The main module does not provide tests for every function; it simply implements the behavior described in the handout. There is no rigorous testing suite included.