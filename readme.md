# Cate: A simple build system
<h3 align="center">
  Cate is a simple build for C/C++ with C-like syntax.
</h3>
<p align="center">
  <img align="center" src="cate_example.png">
</p>

## Introduction
Cate is a simple build system for the C family of languages (minus C#). While not as feature rich as CMake or as fast as Ninja-build, Cate acheives a simple syntax that doesn't feel much different to an actual programming language.

Unlike CMake, Cate is not Turing complete. It doesn't feature if-statements, loops, functions, or anything that is not related to building. 

## Notes
**Cate will change a lot in the following days, look out for updates that *might* break your project**

- Cate was written by a beginner programmer and its codebase is terrible.
- It currently doesn't support threading.
- No Windows support.
- Cate uses robin_hood hashing because it makes Cate 20% faster 

## Building Cate
A Build system needs to be built too. this step will be easy though! 
Dependencies are:
- A *NIX operating system (Linux, BSD, MacOS, etc)
- A C++17 compiler (I used g++)
- GNU Flex 2.6.4 (or greater)
- GNU Make (if you don't have Cate already installed)

### Building with GNU Make
To build with Make, run `make`. or if you want a smaller executable, run `make smol`
### Building with Cate
To build with Make, run `cate build.cate`,  or if you want a smaller executable, run `cate smol.cate`
### Installing
To install, use `sudo ./cate install.cate`, or `sudo make install` if you prefer installing with make.

## How to use Cate
That's a good question we're not sure of yet. Follow this example for a project
```css
Project PROJECT_NAME; /*project declaration*/
PROJECT_NAME.compiler = "g++"; /*required*/
PROJECT_NAME.flags = "-O2"; /*optional*/
PROJECT_NAME.files = {"src/main.cpp", recursive("src/subfolder.cpp")};
PROJECT_NAME.libs = {"ExampleLibrary", "lib/libexample_local_library.so"}; /*optional*/
PROJECT_NAME.out = "outs/example_executable"; /*optional. default is the project name in the current directory*/
PROJECT_NAME.build(); /*will start building*/
```

## Credits
All Milkies have contributed in some way to Cate. Notable contributors are:
- Yogurt (Main maintainer)
- Lime (Tester and bug fixer)
- Lemon (Secondary bug fixer)
- Latte (Feature implementer) 