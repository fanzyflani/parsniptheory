/*
Copyright (c) 2014 fanzyflani. All rights reserved.
CONFIDENTIAL PROPERTY OF FANZYFLANI, DO NOT DISTRIBUTE
*/

#include "common.h"

int sdiv(int n, int d)
{
	if(n >= 0) return n / d;
	else return (n / d) - 1;
}

int smod(int n, int d)
{
	if(n >= 0) return n % d;
	else return d - ((-n) % d);
}


