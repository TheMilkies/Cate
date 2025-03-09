# Cate3 is NOT ready!
Cate3 is a HUGE rewrite in C99 for compatibility reasons. It will have Windows support and less painful platform detection using `if` statements!

It should be as fast or even faster than the older Cate versions since it uses C.

Original Cate by @TheMilkies .

Cate3 Rewrite by @ayinsonu .

# Features that will be added
Most of the development time has gone to testing features and seeing which ones are worth adding. These are 100% going to be added
- Windows support.
- `if` statements for platform checks.
- Built-in `rm, cp, mv, mkdir` for easier compliance.
- More built-in commands such as `print(), error(), exists()`.
- Library type support, you can select whether you want a static or dynamic library with syntax!
- Zero dependencies other than libc.
- Automation can be disabled using `auto = false`
- Small executable size (less than 200KB)
- Strict POSIX and C99 compliance for the best portability (test system is FreeBSD 5.0).
- Custom linker support (without hacks) for OS development.
- Faster execution.
- Better docs in manpages.
- Catel will get better syntax.
- Less clutter (and sadly color), so we could use UNIX utilities with Cate.
- BSD license just to be incompatible with the GPL.

# Features that will be removed
Some things were just not meant to be, and thankfully Cate is small enough that we can make breaking changes that will only affect (at most) 3 people.
- Platform-based catel files.
- The `recursive()` function, you can just use `"*.c"` now!