#!/bin/sh
# POSIX compliant bootstrapping script for cate.
cc="cc"
cflags="-I. -Iinclude -std=c99"
build_dir="cate/build"
out_name="out/cate"
install_to="/usr/local/bin/cate"

is_installed() {
    if ! command -v "$1" 2>&1 >/dev/null; then
        return 1
    fi
    return 0
}

build_init() {
    stat_cmd='stat -f %m'
    os_name=$(uname -o)
    if [ "${os_name#*"GNU"}" != "$os_name" ]; then
        stat_cmd='stat -c %Y' # i hate GNU
    fi
    mkdir -p $build_dir
}

check_newer() {
    [ ! -e "$1" -o ! -e "$2" ] && return 0
    m1=$($stat_cmd "$1")
    m2=$($stat_cmd "$2")
    [ "$m1" -gt "$m2" ] && return 0
    return 1
}

build_file() {
    object=$(echo "$1.o" | tr '/' '_')
    object="$build_dir/$object"
    check_newer $1 $object && $cc $cflags -c $1 -o $object
}

build_init
files=$(ls -1 src/*.c)

for file in $files; do
    build_file $file &
done
wait

mkdir out -p
$cc $cflags $build_dir/*.o -o $out_name

read -p "Install cate? [y/n]: " yn
case $yn in
    [Yy]*)  ;;
    *) exit 0 ;;
esac

root_install() {
    if [ $(id -u) -eq 0 ]; then
        $install_cmd
    else
        echo "Please rerun as root"
        exit 1
    fi
}

if ! command command -v echo 2>&1 >/dev/null; then
    echo "WARNING: This system is not fully POSIX compliant."
    root_install
fi

install_cmd="cp $out_name $install_to"
if is_installed sudo; then
    sudo $install_cmd
elif is_installed doas; then
    doas $install_cmd
else
    root_install
fi
