#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Please insert a version number. like 2.1.0"
	exit
fi

sed -i 's/(Development)/(Release)/g' include/Util.hpp #change Development to Release
touch src/Help.cpp # to make sure it changes
if cate release_smol -t16 -f; then
	sed -i 's/(Release)/(Development)/g' include/Util.hpp #change it back
	cd release
	echo "cp -f cate /usr/bin/cate" >> install.sh #generate the install file
	echo "cp docs/manpages/cate.1 /usr/local/share/man/man1/" >> install.sh #generate the install file
	chmod +x install.sh #add permissons
	tar -czvf linux_cate_v$1.tar.gz cate ../docs/manpages/cate.1 install.sh #create tar
	zip -9 linux_cate_v$1.zip cate install.sh #create zip
	rm install.sh #clean
	cd ..
else
	sed -i 's/(Release)/(Development)/g' src/Util.hpp #change it back
	echo "Build error, bye bye"
fi