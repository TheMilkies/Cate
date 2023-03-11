#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Please insert a version number. like 2.1.0"
	exit
fi

touch src/Help.cpp # to make sure it changes
if cate release_smol -t16 -f; then
	cd release
	echo "cp -f cate /usr/local/bin/cate" >> install.sh #generate the install file
	echo "cp docs/manpages/cate.1 /usr/local/share/man/man1/" >> install.sh #generate the install file
	chmod +x install.sh #add permissons
	tar -czvf linux_cate_v$1.tar.gz cate ../docs/manpages/cate.1 install.sh #create tar
	zip -9 linux_cate_v$1.zip cate install.sh #create zip
	rm install.sh #clean

	deb_cate_name=$(echo "cate_$1_amd64" | sed 's/\(.*\)\./\1-/')
	mkdir -p $deb_cate_name/DEBIAN/ $deb_cate_name/usr/local/bin
	printf "Package: Cate\nVersion: $1\nArchitecture: amd64\nMaintainer: The Milkies <themilkiesyes@gmail.com>\nDescription: Cate: The simple build system for C/C++\n" > $deb_cate_name/DEBIAN/control
	cp cate $deb_cate_name/usr/local/bin/cate
	dpkg-deb --build --root-owner-group $deb_cate_name

	rm -rf $deb_cate_name
	cd ..
else
	echo "Build error, bye bye"
fi