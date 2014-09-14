#!/bin/bash

CFLAGS="-I../../../libartemis/inc/ -std=c99"

if [ "$1" == "debug" ]
CFLAGS="$CFLAGS -O3"
then
CFLAGS="$CFLAGS -g -fstack-check -D _DEBUG"
fi

mkdir obj 2> /dev/null
cp ../../libartemis/gcc4/obj/libartemis.so ./obj/libartemis.so
cd obj

echo gcc $CFLAGS -o artemis ../../src/main.c libartemis.so
gcc $CFLAGS -o artemis ../../src/main.c libartemis.so

cd ..
