# Cate: A simple build system
<h3 align="center">
  Cate is a simple build system for C/C++ with nice syntax.
</h3>
<p align="center">
  <img align="center" src="cate_example.png">
</p>

## Introduction
Cate is a simple build system for the C family of languages (minus C#). While not as feature rich as CMake or as fast as Ninja-build, Cate acheives a simple syntax that doesn't feel much different to an actual programming language.

Unlike CMake, Cate is not Turing complete. It doesn't feature if-stetements, loops, functions, or anything that is not related to building. 

## Notes
**Cate will change a lot in the following days, look out for updates that *might* break your project**

Cate was written by a beginner programmer and its codebase is terrible. It currently doesn't support threading, or the Windows operating system.
I added size optimization flags and such because I ran out of disk space.

## Building Cate
A Build system needs to be built too. this step will be easy though! 
Dependecies are:
- A *NIX operating system (Linux, BSD, MacOS, etc)
- A C++17 compi;er (I used g++)
- GNU Flex 2.6.4 or (greater)
- GNU Make (if you don't have Cate already installed)

### Building with GNU Make
To build with Make, run `make`. or if you want a smaller executaable, run `make smol`
### Building with Cate
To build with Make, run `cate build.cate`,  or if you want a smaller executaable, run `cate smol.cate`
### Installing
To install, use `sudo ./cate install.cate`, or `sudo make install` if you prefer installing with make.

## How to use Cate
That's a good question we're not sure of yet. Follow this example for a project
```css
Project PROJECT_NAME; /*project declarartion*/
PROJECT_NAME.compiler = "g++"; /*required*/
PROJECT_NAME.flags = "-O2"; /*optional*/
PROJECT_NAME.files = {"src/main.cpp", recursive("src/subfolder/*.cpp")};
PROJECT_NAME.libs = {"ExampleLibrary", "lib/libexample_local_library.so"}; /*optional*/
PROJECT_NAME.out = "outs/example_executable"; /*optional. default is the project name in the current directory*/
PROJECT_NAME.build(); /*will start building*/```
