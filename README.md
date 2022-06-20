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

**Step 3:** Build the project. 
```bash
username@machine:~/Differentiator$ make
```