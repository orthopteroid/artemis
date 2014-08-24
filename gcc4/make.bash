#!/bin/bash

CFLAGS="-I../libartemis/inc/ -std=c99"

if [ "$1" == "debug" ]
then
CFLAGS="$CFLAGS -g -fstack-check -D _DEBUG"
fi

mkdir obj 2> /dev/null

echo gcc $CFLAGS -o ./obj/artemis ../src/main.c ../libartemis/gcc4/obj/libartemis.so
gcc $CFLAGS -o ./obj/artemis ../src/main.c ../libartemis/gcc4/obj/libartemis.so

#readelf --symbols ./obj/artemis

if [ "$1" != "debug" ]
then
echo strip -w -K platform_* -K ar_core_* -K ar_uri_* -K test_* obj/artemis
strip -w -K platform_* -K ar_core_* -K ar_uri_* -K test_* obj/artemis
readelf --symbols ./obj/artemis
fi


