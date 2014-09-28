#!/bin/bash

CFLAGS="-fpic -I../inc/ -std=c99 -c -msse3 -Wno-multichar"
#CFLAGS="-fpic -I../inc/ -std=c99 -c"

if [ "$1" == "debug" ]
CFLAGS="$CFLAGS -O3"
then
CFLAGS="$CFLAGS -g -fstack-check -D _DEBUG"
fi

AA=`find ../src -name *.c`

BB=${AA//..\/src\//} # strip path
SRCS=${BB//.c/} # strip ext

ZZ=${AA//..\/src/.\/obj} # swap path
OBJS=${ZZ//.c/.o} # swap ext

mkdir obj 2> /dev/null
for i in $SRCS
do
	echo gcc $CFLAGS -o ./obj/$i.o ../src/$i.c
	gcc $CFLAGS -o ./obj/$i.o ../src/$i.c
	if [ $? -ne 0 ] ; then break ; fi
done

echo gcc -shared -o obj/libartemis.so $OBJS
gcc -shared -o obj/libartemis.so $OBJS

if [ "$1" != "debug" ]
then
echo strip -w -K library_* -K test_* obj/libartemis.so
strip -w -K library_* -K test_* obj/libartemis.so
readelf --symbols ./obj/libartemis.so
fi
