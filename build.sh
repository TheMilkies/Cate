#!/bin/bash
#this script is for when you don't have cate or make installed

CC=cc

if ! command -v $CC &> /dev/null ; then
    echo "No C++ compiler found"
    exit 1
fi

build_folder="cate/build"; mkdir -p $build_folder
out_exec="out/cate"; mkdir -p out
cflags="-Iinclude -lstdc++ -march=native -fpermissive -funsafe-math-optimizations -ffast-math -fno-signed-zeros -ffinite-math-only -std=c++17 -lstdc++fs -Wall -O3 -pthread -ffunction-sections -fdata-sections -Wl,--gc-sections -fno-ident -fomit-frame-pointer -fmerge-all-constants -Wl,--build-id=none"

build_() {
    obj_name="src_$1"
    obj_name_mod="${obj_name/\//"_"}"    

    if [ src/$1.cpp -nt $build_folder/$obj_name_mod.o ]; then
        $CC src/$1.cpp $cflags -c -o $build_folder/$obj_name_mod.o
    fi
}

_build() {
    if ! build_ $1 ; then
        echo "Error in $1" 
        exit 1
    fi
}

_build Parser/Lexer &

_build Class/Class &

_build Parser/Parser &

_build Parser/ParserExpect &

_build Parser/ParserArrays &

_build Parser/Recursive &

_build Util &

wait < <(jobs -p)

_build Class/Library &

_build Class/Project &

_build Class/ClassMethods &

_build Catel &

_build main &

_build Help &

wait < <(jobs -p)

$CC $build_folder/*.o $cflags -o$out_exec 

if ! test -f "./out/cate"; then
    echo "Cate didn't build corectly."
    exit 1
fi

if command -v strip &> /dev/null; then
    strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag out/cate
fi

install_command="cp ./out/cate /usr/local/bin/cate"

read -r -p "Done. Would you like to install Cate? [Y/n]: " response
case "$response" in
    [yY][eE][sS]|[yY]) 
        if command -v doas &> /dev/null; then
            rootc='doas'
        elif command -v sudo &> /dev/null; then
            rootc='sudo'
        elif [ "$EUID" -e 0]; then
            rootc=''
        else
            echo "No way to run as root found, sorry"
            exit 1
        fi
        $rootc $install_command
        $rootc rm -f /usr/bin/cate
        $rootc cp -f docs/manpages/cate.1 /usr/local/share/man/man1/
        ;;
    *)
        exit
        ;;
esac
