# Cate: A Build System For The Sane
<h3 align="center">
  Cate is an easy build system for C/C++ with C-like syntax.
</h3>
<p align="center">
  <img align="center" src="github_stuff/screenshot.png">
</p>

## Introduction
Cate is a really easy and fast build system for C/C++ with readable syntax.

Unlike CMake and other build systems, Cate does not use Make and is not Turing complete. Cate is more like a wrapper state-machine for GCC/clang than an object oriented build system (unlike CMake), or a build system programming language (also unlike CMake).

We believe Cate is the perfect build system for small to medium size projects! It can also be decent for large projects if taken with care.

Do note:
- Cate has been completed as of 3.0!
- This readme is full of little jokes. No offense intended to any other build system... *except Autotools (we hate it so much)*.
- Cate uses Catel, a messy file that allows us to set a default file.
- Cate does not support string splitting.

## Advantages over other build systems
1. It's extremely easy to learn and even guess.
2. It (unlike CMake) doesn't require documentation... but has decent docs.
3. It's smol, it has everything you need and a little more to keep the 1% happy.
4. It's very smart. It can guess and do most things automatically for you.
5. Cate, unlike Make, just cates sense!
6. It's written in pure POSIX.2001 and C99 for portability (it runs on 32bit FreeBSD 5.0 from 2003).

## Installing Cate
### Building from source
Make sure you have these:
- A modern (FreeBSD 5.0 at minimum) POSIX-compliant system.
- A C99 compiler.

#### Using build.sh (Unix-like)
Run `./build.sh`, It'll ask you if you'd like to install at the end.

#### Using Cate (any platform)
**If you're using cate <= 1.3, run `sudo cate legacy`.**

Run `cate`, it'll ask you if to install after building. **Unlike Make and other build systems; it'll automatically detect the thread count.**

## Future
We believe Cate is complete as of 3.0. Updates would only fix bugs or speed it up. Maybe, in the far far future, we would create another build system called "Caden" which would be turing complete and compete with CMake on large-scale projects.

Cate3 is the definitive "Cate". Older versions are depreCATEd and are not considered "valid cate" anymore.

## Credits  
Original Cate by @TheMilkies .

Cate3 Rewrite by @ayinsonu .