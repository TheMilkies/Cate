#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Please insert a version number. like 1.2.8"
	exit
fi

sed -i 's/(Development)/(Release)/g' ../src/Util.hpp #change Development to Release
if cate smol &> /dev/null; then
	sed -i 's/(Release)/(Development)/g' ../src/Util.hpp #change it back
	echo "cp -f out/cate /usr/bin/cate" > install.sh #generate the install file
	chmod +x install.sh #add permissons
	tar -czvf ../out/linux_cate_v$1.tar.gz ../out/cate install.sh #create tar
	zip -9 ../out/linux_cate_v$1.zip ../out/cate install.sh #create zip
	rm install.sh #clean
else
	sed -i 's/(Release)/(Development)/g' ../src/Util.hpp #change it back
	echo "Build error, bye bye"
fi