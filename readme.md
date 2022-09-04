# Cate: A Build System for the sane.
<h3 align="center">
  Cate is a simple build for C/C++ with C-like syntax.
</h3>
<p align="center">
  <img align="center" src="cate_example.png">
</p>

### Table of contents
- [Cate: A Build System for the sane.](#cate--a-build-system-for-the-sane)
  * [Introduction](#introduction)
  * [Notes](#notes)
  * [Building Cate](#building-cate)
    + [Build dependencies](#build-dependencies)
    + [Building with build.sh](#building-with-buildsh)
    + [Building with Cate](#building-with-cate)
    + [Building with GNU Make](#building-with-gnu-make)
    + [Installing](#installing)
  * [How to use Cate](#how-to-use-cate)
    + [Command-line](#command-line)
      - [Flags (options)](#flags--options-)
  * [Syntax](#syntax)
    + [Classes](#classes)
    + [Class properties](#class-properties)
    + [Class methods](#class-methods)
    + [General functions](#general-functions)
  * [Catel](#catel)
    + [Syntax](#syntax-1)
  * [Known issues](#known-issues)
  * [Credits](#credits)
  * [How to contribute](#how-to-contribute)

## Introduction
Cate is a simple build system for the C family of languages (minus C#). While not as feature rich as CMake or as fast as Ninja-build, Cate achieves a simple syntax that doesn't feel much different to C/C++.

Unlike CMake/Make, Cate is not Turing complete. It doesn't feature if-statements, loops, functions, or anything that is not related to building. 

## Notes
- Cate can be just as fast as Make, given the right file count.
- Cate uses Catel, a messy file type that allows default files.
- Cate was written by a beginner programmer and its codebase is quite bad. Fell free to rewrite it if you want!
- No Windows support (yet).
- Cate uses robin_hood hashing, since it's 20% more efficient (on average)
- Cate **does not** support `\"` characters in string literals.

## Building Cate
### Build dependencies
- A *NIX operating system (Linux, BSD, MacOS, etc)
- A C++17 compiler (I used g++)
- GNU Flex 2.6.4 or greater ([read setup here](flex_setup.md)) (not required in x86_64 builds because we included the headers and a static library)

### Building with build.sh
To build with build.sh, run `./build.sh`, it builds the smol cate
### Building with Cate
To build with Cate, run `cate`, it builds the smol cate by default
### Building with GNU Make
To build with Make, run `make`. or if you want a smaller executable, run `make smol`
### Installing
To install use `sudo cp out/cate /usr/bin/cate`, or `sudo make install` if you prefer installing with make.

## How to use Cate
### Command-line
To build another project, run `cate [filename.ending with .cate]` (example: `cate build.cate`).

The `.cate` extension is not required in the command.   (example: `cate build`).

#### Flags (options)
- `-h`: Shows help.
- `-tN`: Changes the thread count to N.
- `-v`: Shows the current installed Cate version.
- `-D`: Disables all `system()` lines in script.  

Cate will only run **one** cate file per command.

For starting a project, look at the [examples folder](examples/) or continue reading.

## Syntax
Cate follows an object syntax. It's very simple to understand for C/C++ programmers. Semicolons and commas are optional.

**IMPORTANT NOTE:** Cate **does not** support `\"` characters in string literals.

### Classes
There are only two classes you can create.
1. `Project`: A class that builds an executable.  (**Example**: `Project proj;`)

2. `Library(type)`: A class that builds a library of specified type (can be `static` xor `dynamic`)  (**Example**: `Library slib(static);`, **Example**: `Library dlib(dynamic);`)

### Class properties
Just like in C/C++, properties follow the `object.property = Thing;`.
**Note:** a class property cannot be set to another's.

Here are the classes' properties:
- `String out` is the output file. Not required (defaults to identifier).
- `String build_directory|object_folder` is the build directory where object files are saved. Not required (defaults to "build/").
- `Array files` holds the filenames of the class's sources. it can be set to an array, an array with `recursive()`, or just `recursive()`. Required. 
- `Array libraries|libs` holds the filenames of the class's libraries. it can only be set to an array. libraries can be either local libraries in a folder or libraries in `/usr/lib`. Not required. 
- `Array includes|include_paths` holds the folder names of the class's includes. Not required.
- `String flags` is the class's compiler flags. Not required. 
- `String final_flags|end_flags` is the final executable compiler flags. Not required. 
- `LibraryType type` is the library's type, can only be `static` pr `dynamic`. Not required since it's already defined in `Library NAME(LibraryType);` 
- `String compiler` is the class's compiler. Required. 

### Class methods
There is **only one** method currently.
- `build()` starts the building process. Can be called twice only in libraries.

### General functions
- `Array recursive(String)` **only in the class's files**: takes a string with a single wildcard (`*`)

Example: `recursive("src/*.cpp")`

## Catel
Catel is complete JANK, but it does its job (somewhat) well! A Catel file is always called `.catel` with no exceptions. it allows you to set have a directory with catefiles in it, and set a default build

### Syntax
Unlike Cate, Catel looks terrible since it doesn't even try to mimic a language. Catel tokenizes with whitespace, meaning you can not have space in the "look-in" directory name or in the default build file name. Catel does not like words in odd numbers so keep your `.catel` nice and even

Take a look at this example:
```
dir cate
default all
```

`dir` is the catefiles directory, Cate will check if the file is there first, if it's not there; Cate will check if the file is in the root directory.

`default` is the default file to build, it can be a path or just the filename (will search `dir` first).

## Known issues
These issues are known and will be fixed soon!
- None for now!

## Credits
<!-- All Milkies have contributed in some way to Cate. Notable contributors are: -->
- Yogurt (Main maintainer)
- Lime (Tester and bug fixer)
- Lemon (Secondary bug fixer)
- Latte (Feature implementer and bug fixer) 

## How to contribute
- Make sure it compiles.
- Make a pull request.
