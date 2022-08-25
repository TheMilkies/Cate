#!/bin/bash
#this script is for when you don't have cate or make installed

if ! command -v g++ &> /dev/null ; then
    echo "g++ is not installd."
    exit
fi

build_folder="cate/build"; mkdir -p $build_folder
cflags="-fpermissive -funsafe-math-optimizations -ffast-math -fno-signed-zeros -ffinite-math-only -std=c++17 -lstdc++fs -Wall -O3 -pthread -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-ident -fomit-frame-pointer -fmerge-all-constants -Wl,--build-id=none -static"

build_() {
    g++ src/$1.cpp $cflags -c -o $build_folder/back_src_$1.o
}

if ! build_ main; then
    exit
fi

if ! build_ Lexer; then
    exit
fi

if ! build_ Class; then
    exit
fi

if ! build_ Parser; then
    exit
fi

if ! build_ Util; then
    exit
fi

if ! build_ Library; then
    exit
fi

if ! build_ Project; then
    exit
fi

g++ $build_folder/*.o externals/linux_libfl.a $cflags -oout/cate
strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/cate