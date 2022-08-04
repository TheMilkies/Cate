# Cate: A simple build system
![](cate_example.png)
## Introduction
Cate is a simple build system for the C family of languages (minus C#). While not as feature rich as CMake or as fast as Ninja-build, Cate acheives a simple syntax that doesn't feel much different to an actual programming language.

Unlike CMake, Cate is not Turing complete. It doesn't feature if-stetements, loops, functions, or anything that is not related to building. 

## Notes
**Cate will change a lot in the following days, look out for updates that *might* break your project**

Cate was written by a beginner programmer and its codebase is terrible. It currently doesn't support threading, or the Windows operating system.
I added size optimization flags and such because I ran out of disk space.

## Building Cate
A Build system needs to be built too. this step will be easy though
Dependecies are:
- A *NIX operating system (Linux, BSD, MacOS, etc)
- A C++17 compi;er (I used g++)
- GNU Flex 2.6.4 or (greater)
- GNU Make (if you don't have Cate already installed)

### Building with GNU Make
To build with Make, run `make`
### Building with Cate
To build with Make, run `cate build.cate`
