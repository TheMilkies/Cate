#!/bin/sh
#POSIX shell script to build Cate for systems without it
set -e
smol=1

CC=cc
cflags="-O3 -march=native -I. -Iinclude -D_DEFAULT_SOURCE -std=c99"
build_folder="cate/build"
out_folder="out"

files=""
get_files() {
    for file in $1/*; do
    if [ -f "$file" ] && [ "${file##*.}" = "c" ]; then
        files="$files $file"
    fi
    done
}

# -nt is not POSIX compliant
is_newer() {
    if [ ! -e "$2" ]; then
        return 0
    fi

    file1_time=$(stat -c %Y "$1")
    file2_time=$(stat -c %Y "$2")

    if [ "$file1_time" -gt "$file2_time" ]; then
        return 0
    else
        return 1
    fi
}

object_files=""
build_file() {
    if is_newer $file $obj_name; then
        $CC -c $file $cflags -o $obj_name
    fi
}

build_files() {
    j=0
    cores=$(getconf _NPROCESSORS_ONLN)
    for file in $@; do
        if [ $j -lt "$cores" ]; then
            obj_name="${file%%.c*}"
            obj_name=$(printf '%s' "$obj_name" | tr '/' '_')
            obj_name="$build_folder/$obj_name.o"
            object_files="$object_files $obj_name"
            build_file $file $obj_name &
        else
            j=0
            wait
        fi
    done
}

get_files src
get_files src/posix
mkdir -p $build_folder $out_folder

build_files $files
wait
$CC $object_files -o $out_folder/cate

if [ "$smol" -eq 1 ]; then
    if ! [ -x "$(command -v strip)" ]; then
        echo "error! no strip command found" >&2
        exit 1
    fi
    strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag $out_folder/cate
fi