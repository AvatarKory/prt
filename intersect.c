
/*
 * intersect.c - The module check for eye/object intersection
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 */

#include <stdio.h>
#include <math.h>
#include "rt.h"
#include "externs.h"

/*
 * Check_and_push()
 * 
 * Check to see of this ray penatrate this bbox around the object. If so, push
 * it on the stack.
 */

void Check_and_push(OBJECT *obj, RAY  *ray)
{
    VECTOR	mn, mx, r_dir, r_org;
    double		t_near, t_far, t1, t2;

    r_dir = ray->dir;
    r_org = ray->pos;

    mn = obj->b_min;
    mx = obj->b_max;

    t_near = -HUGE;
    t_far = HUGE;

    /* test the X slab */
    if (fabs(r_dir.x) < MIN_T)	/* parralel to the X slab */
    {
	if (r_org.x < mn.x || r_org.x > mx.x)
	    return;	/* can't possible hit this puppy */
    }
    else
    {
	/* ray is not parallel to the X slab. calc intersection dist */

	t1 = (mn.x - r_org.x) / r_dir.x;
	t2 = (mx.x - r_org.x) / r_dir.x;

	if (t1 > t2)
	{
	    if (t2 > t_near)
		t_near = t2;
	    if (t1 < t_far)
		t_far = t1;
	}
	else
	{
	    if (t1 > t_near)
		t_near = t1;
	    if (t2 < t_far)
		t_far = t2;
	}

	if (t_near > t_far)
	    return;	/* no hitter			 */

	if (t_far < MIN_T)
	    return;	/* no hitter			 */
    }

    /* test the Y slab */
    if (fabs(r_dir.y) < MIN_T)	/* parralel to the Y slab */
    {
	if (r_org.y < mn.y || r_org.y > mx.y)
	    return;	/* can't possible hit this puppy */
    }
    else
    {
	/* this is not parallel to the Y slab. calc intersection dist */

	t1 = (mn.y - r_org.y) / r_dir.y;
	t2 = (mx.y - r_org.y) / r_dir.y;

	if (t1 > t2)
	{
	    if (t2 > t_near)
		t_near = t2;
	    if (t1 < t_far)
		t_far = t1;
	}
	else
	{
	    if (t1 > t_near)
		t_near = t1;
	    if (t2 < t_far)
		t_far = t2;
	}

	if (t_near > t_far)
	    return;	/* no hitter			 */

	if (t_far < MIN_T)
	    return;	/* no hitter			 */
    }

    /* test the Z slab */
    if (fabs(r_dir.z) < MIN_T)	/* parralel to the Z slab */
    {
	if (r_org.z < mn.z || r_org.z > mx.z)
	    return;	/* can't possible hit this puppy */
    }
    else
    {
	/* ray is not parallel to the Z slab. calc intersection dist */

	t1 = (mn.z - r_org.z) / r_dir.z;
	t2 = (mx.z - r_org.z) / r_dir.z;

	if (t1 > t2)
	{
	    if (t2 > t_near)
		t_near = t2;
	    if (t1 < t_far)
		t_far = t1;
	}
	else
	{
	    if (t1 > t_near)
		t_near = t1;
	    if (t2 < t_far)
		t_far = t2;
	}

	if (t_near > t_far)
	    return;	/* no hitter			 */

	if (t_far < MIN_T)
	    return;	/* no hitter			 */
    }

    /*
     * This object passed all of the test. So this ray will hit this
     * puppy. Push at on the stack.
     */

    Push_object(obj);

}

/*
 * Intersect()
 * 
 * Check to see if given ray intersect any objects. If so, fill in the given
 * intersect structure and return 1. Else, return 0.
 */

int Intersect(RAY *ray, INTERSECT *inter)
{
    int             i, iflag;
    INTERSECT       minter;
    OBJECT         *obj;
    COMPOSITE      *cd;

    iflag = 0;
    /*
     * If the root object is not a slab, then slimply call its inter
     * intersect routine and return.
     */

    if (root->type != T_COMPOSITE)
	return ((*root->inter) (root, ray, inter));

    /*
     * Push root node an top of stack and check to set if we hit
     * anything.
     */

    stack_cnt = 0;

    Check_and_push(root, ray);

    while (stack_cnt != 0)
    {

	obj = Pop_object();

	/*
	 * If this object is a composite type, then check and push
	 * all of its childeren onto the stack.
	 */


	if (obj->type == T_COMPOSITE)
	{
	    cd = (COMPOSITE *) obj->obj;
	    for (i = 0; i < cd->num; i++)
		Check_and_push(cd->child[i], ray);
	}
	else
	{

	    if ((*obj->inter) (obj, ray, inter))
	    {
		if (iflag == 0)	/* first intersection */
		{
		    iflag = 1;
		    minter.t = inter->t;
		    minter.obj = inter->obj;
		}
		else if (minter.t > inter->t)
		{
		    minter.t = inter->t;
		    minter.obj = obj;
		}
	    }
	}
    }

    if (iflag)
    {
	inter->t = minter.t;
	inter->obj = minter.obj;
	return (1);
    }
    else
	return (0);

}

