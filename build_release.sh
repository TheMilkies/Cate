#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Please insert a version number. like 2.1.0"
	exit
fi

touch src/Help.cpp # to make sure it changes
if cate release_smol -t16 -f; then
	cd release
	echo "cp -f cate /usr/bin/cate" >> install.sh #generate the install file
	echo "cp docs/manpages/cate.1 /usr/local/share/man/man1/" >> install.sh #generate the install file
	chmod +x install.sh #add permissons
	tar -czvf linux_cate_v$1.tar.gz cate ../docs/manpages/cate.1 install.sh #create tar
	zip -9 linux_cate_v$1.zip cate install.sh #create zip
	rm install.sh #clean
	cd ..
else
	echo "Build error, bye bye"
fi