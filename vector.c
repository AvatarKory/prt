
/*
 * vector.c - This module contains all of the vector math related functions.
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 */

#include <stdio.h>
#include <math.h>

#include "rt.h"

double 
VecNormalize(VECTOR *v)
{
	double          len;

	len = VecLen(*v);
	v->x /= len;
	v->y /= len;
	v->z /= len;

	return (len);
}
