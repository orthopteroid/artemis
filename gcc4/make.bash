#!/bin/bash

gcc -I../libartemis/inc/ -std=c99 -o ./obj/artemis ../src/main.c  ../libartemis/gcc4/obj/libartemis.a

readelf --symbols ./obj/artemis

