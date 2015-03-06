/*
 * sphere.c - This module contain all of the code that relates to spheres.
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
 */

#include <stdio.h>
#include <malloc.h>
#include <math.h>

#include "rt.h"
#include "externs.h"

int            Sphere_intersect();
void		Sphere_normal();

/*
 * Build_sphere()
 * 
 * Given some info on a sphere object, build a complete object stucture.
 */

void Build_sphere(SPHERE         *s)
{
    OBJECT         *o;

    if (nobjects == MAX_PRIMS)
    {
	fprintf(stderr, "%s: too many objects specified\n", my_name);
	exit(1);
    }

    if ((o = (OBJECT *) malloc(sizeof(OBJECT))) == NULL)
    {
	fprintf(stderr, "%s: malloc failed\n", my_name);
	exit(1);
    }

    s->radius2 = s->radius * s->radius;

    o->type = T_SPHERE;
    o->obj = s;
    o->surf = cur_surface;
    o->inter = Sphere_intersect;
    o->normal = Sphere_normal;

    objects[nobjects++] = o;

    /*
     * Setup of bounding box for this puppy.
     */

    o->b_min.x = s->center.x - s->radius;
    o->b_min.y = s->center.y - s->radius;
    o->b_min.z = s->center.z - s->radius;

    o->b_max.x = s->center.x + s->radius;
    o->b_max.y = s->center.y + s->radius;
    o->b_max.z = s->center.z + s->radius;

}


/*
 * Sphere_intersect()
 * 
 * Check given sphere for intersection with given ray. Return TRUE if an
 * intersection takes place.
 */

int Sphere_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    SPHERE         *s;
    VECTOR          oc;
    double          l2oc, tca, t2hc, disc;
    double          t;

    s = obj->obj;

    /* calculate the origin to center vector */

    VecSub(s->center, ray->pos, oc);
    l2oc = VecDot(oc, oc);

    /* find out the closest approach along the ray */
    tca = VecDot(oc, ray->dir);
    t2hc = s->radius2 - l2oc + (tca * tca);

    /* if the discriminator < 0, then the ray will not hit */
    if (t2hc < MIN_T)
	return (0);

    disc = sqrt(t2hc);

    /* if ray is inside object, set the inside flag */
    if (l2oc > s->radius2 + MIN_T)
    {
	inter->inside = 0;
	t = tca - disc;
    }
    else
    {
	inter->inside = 1;
	t = tca + disc;
    }

    if (t < MIN_T)
	return (0);

    inter->obj = obj;
    inter->t = t;

    return (1);

}

/*
 * Sphere_normal()
 * 
 * Return the normal to a sphere at a given point along the surface.
 */

void Sphere_normal(SPHERE *sphere, RAY *ray, VECTOR *ip, VECTOR *normal)
{
    VecSub(*ip, sphere->center, *normal);
    VecNormalize(normal);
    if (VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}
