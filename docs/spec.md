# Cate specification
You may be wondering why this is a Thing... well we just had a few hours off lmao. Well this isn't fully complete but tells you enough about Cate to make your own implementation if you'd ever like.

## Notation in this document
- We'll use `$` for "this instance's property".
- The symbol `|` is a name separator, since a property can have many names.

## Classes
Cate must support only two class types, being `Project` and `Library` which are derived from the `Class` superclass.

### Class
### Public properties:
- `Array<String> files`: Files of the project/library. No default.
- `Array<String> incs|includes|include_paths`: Include paths of the project/library. Defaults to `"include/"` or `"inc/"` if present in the current directory.
- `Array<String> defs|defines|definitions`: Definitions. Default is set by the compiler. 
- `String out`: The output file name. 
- - In projects: Defaults to the identifier.
- - In libraries: Defaults to "lib" + the identifier + the extension for the library type.
- `String cc|compiler`: The compiler to use. Default is `cc`.
- `String std|standard`: The C/C++ standard to use. Default is set by the compiler.
- `String obj_dir|object_dir|build_dir|build_directory`: The folder it'd store object files in. Defaults to `"build"` (or `cate/build` if catedir is present), unless a directory named `"obj"` is present; where it will use it.
- `String flags`: The cflags of the project/library, All object files are compiled with them. Default is empty.
- `String final_flags`: The cflags ran at the end (linking step (still done by the compiler)) of the project/library's compilation. Default is empty.

- `bool link`: Whether to run the linking step (compile all object files together) or not. Default is `true`.
- `bool threading`: Whether to add `-pthread` to build command. Default is `false`. (Just syntactical sugar.)
- `bool smol|smolize`: Whether to attempt to reduce output-file's size with minimal to no performance loss. Default is `false`. Read [smol](#smol)

#### Private properties:
- `String name`: The instance name, must be the identifier given by the user.

#### Purely virtual functions
These MUST be overridden by the subclasses. 
- `void build()`: Builds project/library.
- `void clean()`: Deletes project/library's object files, should NOT affect other projects/libraries.
- `void install()`: Install projects to `/usr/local/bin` and libraries to `/usr/local/lib`.

### LibraryType
A `LibraryType` can only be `static` or `dynamic`.

### Library
A `Library` has a `LibraryType` variable that corresponds to how it will be built. A new `Library` declaration must be done as follows: `Library NAME(LibraryType)`. 

All object files of libraries must have the `"-g -shared"` flags added automatically to their build command. 

A `Library`'s type can be changed by the user by using the following syntax: `NAME.type = LibraryType`

A `Library`'s `$out` will be automatically generated at after every type change (including when the library was first defined). This is how $out should be generated unless given a different value by the user:
- When `static`: "lib" + `$name` + ".a"
- When `dynamic`: "lib" + `$name` + ".so" (or ".dll" when running on Windows)

### Project
A `Project` will generate an executable, it does not have a `LibraryType` variable since it is not a library lol. A new `Project` is declared as so: `Project NAME` .

A `Project`'s `$out` will be automatically set to the instance's `$name` when the user defines the instance, This is how $out should be generated unless given a different value by the user:
- On Windows: `$name` + ".exe"
- On UNIX-like: `$name`

## Syntax
To save time and retyping, we use a `current_class` **pointer**.
### Declarations
- Project are declared as so: `Project NAME` .
- Libraries are declared as so: `Library NAME(LibraryType)` .

Set `current_class` to the newly defined class.

### Setting
When an instance name is used (`NAME.PROPERTY`): set `current_class` to it and continue. The class is implied from now on.

Cate does not support "getting", this syntax is illegal: `a.flags = b.flags;`

Setting is done like so `.PROPERTY = NEW_VALUE` . It will depend on if the `PROPERTY` is an array or a string.

Example: `.flags = "-O3"`

### Array syntax
Exactly how C handles arrays, though commas are completely optional and properties can not be given. A special function named `recursive()` (or `iterate()`) can be used too.

In `$libs`, an identifier of a previously defined `Library` instance can be given. append the `Library`'s "out" string to the `$files` array.

If a string has an `*` , call `recursive()` with that string.

Example: `.libs = {"GL", LocalLib, "out/*.a"}`

### Recursive
`Array<String> recursive(String path)`: generally expects a string like this: `recursive("dir/*.extension")` . It can be used on `$files` and `$libs` .

It must not allow `"dir/*/*.EXTENSION"` , but allow `"dir/**.EXTENSION"` subrecursion.

## smol
Add size optimization flags automatically if set true, then a strip command.

The performance of the output **must not be impacted**. 

## Initing
The `-i` flag takes a name argument and creates:
- 3 folders: `src cate include`
- 4 files: `src/main.cpp cate/debug.cate cate/release.cate .catel`
Notes:
- The cate files shall build a simple C++ project
- The catel file shall point at the `cate` directory and the debug file.
