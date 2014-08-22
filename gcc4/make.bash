#!/bin/bash

mkdir obj 2> /dev/null

gcc -g -I../libartemis/inc/ -std=c99 -o ./obj/artemis ../src/main.c  ../libartemis/gcc4/obj/libartemis.so

#readelf --symbols ./obj/artemis

