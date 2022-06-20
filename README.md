![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)

# Differentiator

This project is the pre-last task of the programming course by [Ilya Dedinsky (aka ded32)](https://github.com/ded32).

## General information

**Differentiator** accepts a .txt file as input. This file should contain a math function of one or many variables such as
```
x ^ (5 + tan (1 / x))
```
or
```
sin (15 * x) + cos (ln (x ^ y))
```

The output is some .png files with binary trees of the original function and its partial derivatives and a .tex file with formulas of all mentioned functions. Binary trees are made by Graphviz.

## Build and run

**Differentiator** is released for Linux only.

**Step 1:** Clone this repository.
```bash
git clone git@github.com:KetchuppOfficial/Differentiator.git
cd Differentiator
```

**Step 2:** Clone submodule.
```bash
git submodule init
git submodule update
```

**Step 3:** Install Graphviz if it's not installed
```bash
sudo apt install graphviz
```

**Step 4:** Build the project. 
```bash
username@machine:~/Differentiator$ make
Collecting dependencies for "src/Differentiator.c"...
Collecting dependencies for "src/Graphic_Dump.c"...
Collecting dependencies for "src/Stack.c"...
Collecting dependencies for "src/Parser.c"...
Collecting dependencies for "src/Lexer.c"...
Collecting dependencies for "src/main.c"...
Compiling "src/main.c"...
Compiling "src/Lexer.c"...
Compiling "src/Parser.c"...
Compiling "src/Stack.c"...
Compiling "src/Graphic_Dump.c"...
Compiling "src/Differentiator.c"...
Collecting dependencies for "src/My_Lib.c"...
Compiling "src/My_Lib.c"...
ar: creating My_Lib.a
Linking project...
```

**Step 5:** Running
```bash
make run IN=input_file_name
```
The program won't work if you don't specify **input_file_name**.

## Examples

    1) f(x) = x ^ x

Function:

![x_pow_x](/examples/x_pow_x.png)

Derivative of x:

![x_pow_x_der](/examples/x_pow_x_der.png)

    2) f(x, y) = y * sin (x / 5) + y ^ cos (x * y)

Function of x:

![foo_of_x](/examples/Function_Of_x.png)

Function of xy:

![foo_of_y](/examples/Function_Of_y.png)

Derivatives of x:

![der_of_x](/examples/Partial_Derivative_Of_x.png)

Derivatives of y:

![der_of_y](/examples/Partial_Derivative_Of_y.png)
