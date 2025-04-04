# Cate: A Build System for the sane.
<h3 align="center">
  Cate is a simple build system for C/C++ with C-like syntax.
</h3>
<p align="center">
  <img align="center" src="github_stuff/cate_example.png">
</p>

## Introduction
Cate is a simple and fast build system for C/C++, its syntax is much simpler than the other build systems. While Cate is slower than Make, it's much easier to set up and create projects and libraries with.

**IMPORTANT: Cate3 is being worked on, it will bring Windows support and support many more features. It will be the last version of Cate so we're making sure it will have everything we need.**

Unlike CMake and other build systems, Cate does not require Make and **is not** Turing complete. Cate is more like a wrapper state-machine for GCC/clang than an object oriented build system (unlike CMake), or a build system programming language (also unlike CMake).

Cate is not written in Rust and never will be; Cate has 0 memory leaks thanks to a practice known as "knowing how memory works".

Do note:
- Cate uses Catel, a messy file that allows us to set a default file.
- Cate **does not** support string splitting.
- This readme is full of little jokes. No offense intended to any other build system... *except Autotools*.

## Advantages of Cate over other build systems
You may be wondering what issues Cate solves, let us clear it up for you!
1. It's extremely easy to learn, it doesn't require learning an entirely new language just to build a project!
2. It (unlike CMake) has a consistent syntax that doesn't require documentation.
3. It's smol, it has everything it needs and a little more to keep the 1% happy.
4. It's colorful and fun to use, not everything has to be monochrome.
5. Cate, unlike Make, just cates sense!

## Installing Cate
If you're still here; that means you suffered enough CMake (or Autotools) to reconsider your life choices, Thank you for choosing Cate!

### Debian/Ubuntu
Run the following commands:
```sh
wget https://github.com/TheMilkies/Cate/releases/download/v2.9.6/cate_2.9-6_amd64.deb
sudo dpkg -i cate_2.9-6_amd64.deb
rm cate_2.9-6_amd64.deb
```

### Other distributions
Run the following commands:
```sh
mkdir catering
cd catering
wget https://github.com/TheMilkies/Cate/releases/download/v2.9.6/linux_cate_v2.9.6.zip
unzip linux_cate_v2.9.6.zip
sudo ./install.sh
cd ..
rm -rf catering
```

## Building from source
Make sure you have these installed:
- A Unix-like operating system
- A C++17 compiler (`g++` or `clang++`)

### Using build.sh
Run `./build.sh`, It'll ask you if you'd like to install at the end.

### Using Cate
**If you're using cate <= 1.3, run** `sudo cate legacy`.

Run `cate`, it'll ask you if to install after building.
Unlike Make and other build systems; **it'll automatically detect the thread count.**

## Using Cate
Cate's CLI is intuitive, but doesn't offer much more than necessary.

### Flags (Options)
- `-l`: Lists Catefiles in Catefiles directory (set by Catel).
- `-iV`: Init a project with the name **V**.
- `-tN` (and `-jN`): Set thread count to **N**. Cate automatically detects thread count so this isn't required.
- `-y`: Install without asking (always answer 'y').
- `-n`: Don't install (always answer 'n').
- `-D`: Disable all user-defined `system()` calls in script.
- `-d`: Print all commands in script without running them. (dry run)
- `-S`: Smolize even if not set in script.
- `-f` (and `-B`): Forcefully rebuild project.
- `-v`: Shows the installed Cate version. 
- `-h`: Shows help and Cate version. 
- `-A`: **PLEASE DO NOT USE THIS.** Disables the useless security measure.

## Creating a Cate project
Create the following structure
```
cate/
  |_ build.cate

include/

src/
  |_ main.c
```

Or use the following commands
```sh
mkdir cate include src
touch cate/build.cate src/main.c
```

## Creating Catefiles (Catering)
You've come this far! Good Job!

Cate breaks most known build-system conventions by forcing you to use multiple files for different targets, and having a file extension (unlike CMake, Make, Autotools, and many more). For a debug build you'll have a `debug.cate`, for a cross-platform build you'll have a `platformname.cate`. 

### Syntax
Cate uses C-like syntax with the exception of it being a "state-machine" rather than a language. It does not support int-literals (0123456789) as of yet (and hopefully forever). Cate supports `#comments` in addition to C-comments. 

**Cate does not support** `a.property = b.property;` **syntax**

There are only two class types, `Project` and `Library`. 

Example project
```css
Project project;
project.files = {"src/main.c"};
project.includes = {"include"};
project.libs = {/*add libraries here*/};
project.flags = "/*flags here*/";
project.smol = true;
project.out = "/*out name here*/";

project.build();
```

Libraries require a parameter called `LibraryType` which can be either `static` or `dynamic`

Example library (not in example project)
```css
Library library(static)
library.files = {"src/main.c"};
library.includes = {"include"};
library.libs = {/*add libraries here*/};
library.flags = "-O2";
library.out = "out/liblibrary.a";

library.build();
```

Cate (since 2.6) does not require the object names to be repeated.
```css
Project proj;
.flags = "-O3";
.files = "src/main.c";
.build();
```

### Properties
Both classes have these properties, even if they don't make sense for the class

- `Array<String> files`: Files of the project/library. No default.
- `Array<String> incs|includes|include_paths`: Include paths of the project/library. Defaults to `"include/"` or `"inc/"` if present.
- `Array<String> defs|defines|definitions`: Definitions. Default is set by the compiler. 

- `String out`: The output file name. 
  - In projects: Defaults to the identifier.
  - In libraries: Defaults to "lib" + the identifier + the extension for the library type.
- `String cc|compiler`: The compiler to use. Default is `cc`.
- `String std|standard`: The C/C++ standard to use. Default is set by the compiler.
- `String obj_dir|object_dir|build_dir|build_directory`: The folder it'd store object files in. Defaults to `"build"` (or `cate/build` if catedir is present), unless a directory named `"obj"` is present; where it'd use it.
- `String flags`: The cflags of the project/library, All object files are compiled with them. Default is empty.
- `String final_flags`: The cflags ran at the end (linking step) of the project/library's compilation. Default is empty.

- `bool link`: Whether to run the linking step or not. Default is `true`.
- `bool threading`: Whether to add `-pthread` to build command. Default is `false`. (Just syntactical sugar.)
- `bool smol|smolize`: Whether to attempt to reduce output-file's size with minimal to no performance loss. Default is `false`. Do **NOT** use with libraries.

- `LibraryType type`: Type of library, `static` or `dynamic`. Gets from library "constructor".

### Class methods
- `void build()`: Builds project/library.
- `void clean()`: Deletes project/library's object files, doesn't affect other projects/libraries!
- `void install()`: Install projects to `/usr/local/bin` and libraries to `/usr/local/lib`.

### General functions
- `Array<String> recursive(String path)`: Get all files (or libraries, or include paths) in path ending with an extension. Example: `project.files = recursive("src/*.c");`. 
  - `recursive()` Allows subdirectory recursion, Example: `recursive("src/**.c")`;
  - `recursive()` is also called `iterate()`.
  - If for some reason you don't have enough disk space to type `recursive`, you can do `files = {"src/*.c"}`

- `void system(String command)`: Run command. Will be skipped if user runs Cate with the `-D` flag.
- `void subcate(String file_name)`: Starts a new Cate "instance" with the passed file name. (since 2.7)
- `void mkdir(String)`: Create a new directory at the specified path. (since 2.9.6)

### Subcate (fully functional since 2.9.5)
Cate allows you to run another catefile from the current one and use libraries that you built in the other.

`static.cate`
```java
Library example_lib(static)
.files = recursive("src/lib/*.c")
.build() //will be named "out/libexample_lib.a" automatically
```

`test.cate`
```java
subcate("static.cate") //include it here
Project Test
.libs = {example_lib} //we can use it here
.files = recursive("src/test.c")
.build()
```

### Global (since 2.8.1)
All classes use global values as default. There are only 5 global variables you can change, being:
- `String cc|compiler`
- `String std|standard`
- `String obj_dir|object_dir|build_dir|build_directory`
- `bool smol|smolize`
- `bool threading`

Usage example:
```c
compiler = "g++" //global
smol     = true

Project proj
.flags = "-O3"
.files = {"src/main.cpp"}
.build()

Project proj2
.flags = "-O3"
.files = {"src2/main.cpp"}
.build()

Project proj3
.compiler = "cc"
.flags = "-O3"
.files = {"src3/main.c"}
.build()
.install()
```

### Catel
A Catel file (`.catel`) is a dumb file made to point cate at the right directory, and use a default file.

Since 2.8.1; you can create Catel files named:
- `.linux.catel`
- `.mac.catel`
- `.windows.catel`

for those targets.

Here's an example Catel file:
```py
def smol.cate
dir cate
```

Here's an example of Catel for different platforms:

`.windows.catel`
```py
dir cate/windows
def smol
```
`.linux.catel`
```py
dir cate/linux
def smol
```

## Credits
- Yourt
- AyinSonu (cate3)

## Special thanks
Special thanks to
- **Make** for being hard to work with, and extremely ugly.
- **CMake** for failing to be an an improvement over make, and becoming so complicated over the years that you need a **CMake debugger** to use it.
- **Autotools** for being the worst build system to ever exist. 

Without these crimes against humanity, Cate would not have existed.

Thank you; Make, CMake, and Autotools for being so terrible.
