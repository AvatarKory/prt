
/*
 * cone.c
 * 
 * This module conatins all of the code which relates to cones and cylinders.
 * 
 * I must admit, MTV's code helped to "inspire" this module.
 *
 * Copyright (C) 1990-2015, Kory Hamzeh.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License V3
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <math.h>
#include "rt.h"
#include "externs.h"


int             Cone_intersect();
void		Cone_normal();

void Build_cone(CONE *cd)
{
    OBJECT         *obj;
    double          dmin, dmax, d, ftmp;
    VECTOR          tmp;

    if (nobjects == MAX_PRIMS)
    {
	fprintf(stderr, "%s: too many objects specified\n", my_name);
	exit(1);
    }

    if ((obj = (OBJECT *) malloc(sizeof(OBJECT))) == NULL)
    {
	fprintf(stderr, "%s: malloc failed\n", my_name);
	exit(1);
    }

    objects[nobjects++] = obj;

    obj->type = T_CONE;
    obj->inter = Cone_intersect;
    obj->normal = Cone_normal;
    obj->surf = cur_surface;

    VecSub(cd->apex, cd->base, cd->w);
    cd->height = VecNormalize(&cd->w);
    cd->slope = (cd->apex_radius - cd->base_radius) / (cd->height);
    cd->base_d = -VecDot(cd->base, cd->w);

    tmp.x = 0;
    tmp.y = 0;
    tmp.z = 1;

    if (1.0 - fabs(VecDot(tmp, cd->w)) < MIN_T)
    {
	tmp.y = 1;
	tmp.x = 0;
    }

    /*
     * find two axes which are at right angles to cone_w
     */

    VecCross(cd->w, tmp, cd->u);
    VecCross(cd->u, cd->w, cd->v);

    VecNormalize(&cd->u);
    VecNormalize(&cd->v);

    cd->min_d = VecDot(cd->w, cd->base);
    cd->max_d = VecDot(cd->w, cd->apex);

    if (cd->max_d < cd->min_d)
    {
	ftmp = cd->max_d;
	cd->max_d = cd->min_d;
	cd->min_d = ftmp;
    }

    obj->obj = cd;

    /*
     * Create the bounding box for this puppy.
     */

    dmin = HUGE;
    dmax = -HUGE;

    /* first the X plane */
    d = cd->base.x - cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->base.x + cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.x - cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.x + cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    obj->b_min.x = dmin;
    obj->b_max.x = dmax;

    /* now the Y plane */
    dmin = HUGE;
    dmax = -HUGE;

    d = cd->base.y - cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->base.y + cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.y - cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.y + cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    obj->b_min.y = dmin;
    obj->b_max.y = dmax;

    /* and finally the Z plane */
    dmin = HUGE;
    dmax = -HUGE;

    d = cd->base.z - cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->base.z + cd->base_radius;

    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.z - cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    d = cd->apex.z + cd->apex_radius;
    if (d < dmin)
	dmin = d;
    if (d > dmax)
	dmax = d;

    obj->b_min.z = dmin;
    obj->b_max.z = dmax;
}


int Cone_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    RAY             tray;
    CONE           *cd;
    VECTOR          v, p;
    double          a, b, c, d, disc;
    double          t1, t2;
    int             nroots;

    cd = (CONE *) (obj->obj);

    /*
     * First, we get the coordinates of the ray origin in the objects
     * space....
     */

    VecSub(ray->pos, cd->base, v);

    tray.pos.x = VecDot(v, cd->u);
    tray.pos.y = VecDot(v, cd->v);
    tray.pos.z = VecDot(v, cd->w);

    tray.dir.x = VecDot(ray->dir, cd->u);
    tray.dir.y = VecDot(ray->dir, cd->v);
    tray.dir.z = VecDot(ray->dir, cd->w);

    a = tray.dir.x * tray.dir.x
	+ tray.dir.y * tray.dir.y
	- cd->slope * cd->slope * tray.dir.z * tray.dir.z;

    b = 2.0 * (tray.pos.x * tray.dir.x + tray.pos.y * tray.dir.y -
	       cd->slope * cd->slope * tray.pos.z * tray.dir.z
	       - cd->base_radius * cd->slope * tray.dir.z);

    c = cd->slope * tray.pos.z + cd->base_radius;
    c = tray.pos.x * tray.pos.x + tray.pos.y * tray.pos.y - (c * c);

    disc = b * b - 4.0 * a * c;

    if (disc < 0.0)
	return (0);

    disc = sqrt(disc);
    t1 = (-b - disc) / (2.0 * a);
    t2 = (-b + disc) / (2.0 * a);

    if (t2 < MIN_T)
	return (0);

    if (t1 < MIN_T)
    {
	nroots = 1;
	t1 = t2;
    }
    else
    {
	nroots = 2;
    }

    /*
     * ensure that the points are between the two bounding planes...
     */

    switch (nroots)
    {
    case 1:
	VecAddS(t1, ray->dir, ray->pos, p);
	d = VecDot(cd->w, p);

	if (d >= cd->min_d && d <= cd->max_d)
	{
	    inter->t = t1;
	    inter->obj = obj;
	    inter->inside = 1;
	    return (1);
	}
	else
	{
	    return (0);
	}
	break;

    case 2:
	VecAddS(t1, ray->dir, ray->pos, p);
	d = VecDot(cd->w, p);

	if (d >= cd->min_d && d <= cd->max_d)
	{
	    inter->t = t1;
	    inter->obj = obj;
	    inter->inside = 0;
	    return (1);
	}
	else
	{
	    VecAddS(t2, ray->dir, ray->pos, p);
	    d = VecDot(cd->w, p);
	    if (d >= cd->min_d && d <= cd->max_d)
	    {
		inter->t = t2;
		inter->obj = obj;
		inter->inside = 1;
		return (1);
	    }
	}
	return (0);
    }
    return (0);
}


void Cone_normal(CONE *cone, RAY *ray, VECTOR *ip, VECTOR *normal)
{
    VECTOR          v;
    double          t;

    /*
     * fill in the real normal... Project the point onto the base plane.
     * The normal is a vector from the basepoint through this point, plus
     * the slope times the cone_w vector...
     */

    t = -(VecDot(*ip, cone->w) + cone->base_d);
    VecAddS(t, cone->w, *ip, v);
    VecSub(v, cone->base, *normal);
    VecNormalize(normal);
    VecAddS(-cone->slope, cone->w, *normal, *normal);
    VecNormalize(normal);
    if (VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}
