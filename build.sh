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
    if [ src/$1.cpp -nt $build_folder/src_$1.o ]; then
        $CC src/$1.cpp $cflags -c -o $build_folder/src_$1.o
    fi
}

_build() {
    if ! build_ $1 ; then
        echo "Error in $1" 
        exit 1
    fi
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

_build Catel &

wait < <(jobs -p)

$CC $build_folder/*.o externals/linux_amd64_libfl.a  $cflags -o$out_exec 

if ! test -f "./out/cate"; then
    echo "Cate didn't build corectly."
    exit 1
fi

if command -v strip &> /dev/null; then
    strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/cate
fi

install_command="cp ./out/cate /usr/bin/cate"

read -r -p "Done. Would you like to install Cate? [Y/n]: " response
case "$response" in
    [yY][eE][sS]|[yY]) 
        if command -v doas &> /dev/null; then
            sudo $install_command
        elif command -v sudo &> /dev/null; then
            doas $install_command
        elif [ "$EUID" -e 0]; then
            $install_command
        else
            echo "No way to run as root found, sorry"
        fi
        ;;
    *)
        exit
        ;;
esac
