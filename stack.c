/*
 * stack.c
 * 
 * This module conatains all of the code for the object intersect test stack.
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 */

#include <stdio.h>
#include <math.h>

#include "rt.h"
#include "externs.h"

/*
 * Push_object()
 * 
 * Push the object onto the stack. Die of stack overflows.
 */

void Push_object(OBJECT *obj)
{

    /* check to stack overflow */
    if (stack_cnt == STACK_SIZE)
    {
	fprintf(stderr, "%s: object stack overflow\n", my_name);
	exit(1);
    }

    /* push it !! */
    object_stack[stack_cnt++] = obj;
}

/*
 * Pop_object()
 * 
 * Pop an object from the stack. If none exist, die.
 */

OBJECT         *
Pop_object()
{

    /* check for stack undeflow */
    if (stack_cnt == 0)
    {
	fprintf(stderr, "%s: object stack underflow\n", my_name);
	exit(1);
    }

    return (object_stack[--stack_cnt]);

}
