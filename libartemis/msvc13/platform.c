// Copyright 2014 John Howard (orthopteroid@gmail.com)

//#include <crtdefs.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

unsigned char *strndup(const char *s, size_t n)
{
	char* p = malloc( n + 1 );
	memcpy( p, s, n );
	p[n] = 0;
	return p;
}
