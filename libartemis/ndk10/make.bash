#!/bin/bash

#export PATH=~/android-toolchain/bin/:$PATH
#export CC="arm-linux-androideabi-gcc"
#export CXX="arm-linux-androideabi-g++"

AA=`find ../src -name *.c`

BB=${AA//..\/src\//} # strip path
SRC=${BB//.c/} # strip ext

ZZ=${AA//..\/src/.\/obj} # swap path
OBJS=${ZZ//.c/.o} # swap ext

echo $SRC
echo $OBJS

for i in $SRC
do
	echo gcc -I../inc/ -std=c99 -c -o ./obj/$i.o ../src/$i.c
	gcc -I../inc/ -std=c99 -c -o ./obj/$i.o ../src/$i.c
	if [ $? -ne 0 ] ; then break ; fi
done

echo ld -lc -o obj/libartemis.a $OBJS
ld -lc -o obj/libartemis.a $OBJS
strip -w -K platform_* -K ar_core_* -K ar_uri_* obj/libartemis.a

# https://sites.google.com/site/malvanos/tutorials/static-linking-with-gcc
# https://www.technovelty.org/linux/stripping-shared-libraries.html

#nm -g obj/libartemis.a  | grep " U"
readelf --symbols ./obj/libartemis.a

