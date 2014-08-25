#!/bin/bash

CFLAGS="-fpic -I../inc/ -std=c99 -c -msse3"
#CFLAGS="-fpic -I../inc/ -std=c99 -c"

if [ "$1" == "debug" ]
then
CFLAGS="$CFLAGS -g -fstack-check -D _DEBUG"
fi

AA=`find ../src -name *.c`

BB=${AA//..\/src\//} # strip path
SRC=${BB//.c/} # strip ext

ZZ=${AA//..\/src/.\/obj} # swap path
OBJS=${ZZ//.c/.o} # swap ext

mkdir obj 2> /dev/null
for i in $SRC
do
	echo gcc $CFLAGS -o ./obj/$i.o ../src/$i.c
	gcc $CFLAGS -o ./obj/$i.o ../src/$i.c
	if [ $? -ne 0 ] ; then break ; fi
done
echo gcc -shared -o obj/libartemis.so $OBJS
gcc -shared -o obj/libartemis.so $OBJS

if [ "$1" != "debug" ]
then
echo strip -w -K platform_* -K ar_core_* -K ar_uri_* -K test_* obj/libartemis.so
strip -w -K platform_* -K ar_core_* -K ar_uri_* -K test_* obj/libartemis.so
readelf --symbols ./obj/libartemis.so
fi
