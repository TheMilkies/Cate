#!/bin/bash
#this script is for when you don't have cate or make installed

CC=cc

if ! command -v $CC &> /dev/null ; then
    echo "no C++ compiler installed."
    exit 1
fi

build_folder="cate/build"; mkdir -p $build_folder
out_exec="out/cate"; mkdir -p out
cflags="-lstdc++ -march=native -fpermissive -funsafe-math-optimizations -ffast-math -fno-signed-zeros -ffinite-math-only -std=c++17 -lstdc++fs -Wall -O3 -pthread -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-ident -fomit-frame-pointer -fmerge-all-constants -Wl,--build-id=none"

build_() {
    $CC src/$1.cpp $cflags -c -o $build_folder/src_$1.o
}

_build() {
    if ! build_ $1 ; then exit 1; fi
}

_build Lexer &

_build Class &

_build Parser &

_build ParserExpect &

_build Recusive &

_build Util &

_build Library &

_build Project &

_build ClassMethods &

wait < <(jobs -p)

$CC $build_folder/*.o $cflags -o$out_exec externals/linux_amd64_libfl.a
if command -v strip &> /dev/null; then
    strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/cate
fi