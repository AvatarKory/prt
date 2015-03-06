
/*
 * ring.c - This module conatins all of the code that relates to the ring
 * primitive.
 * 
 * Copyright (C) 1990-2015, Kory Hamzeh
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
#include <stdlib.h>
#include <math.h>

#include "rt.h"
#include "externs.h"

int             Ring_intersect();
void		Ring_normal();
extern int      line;

/*
 * Build_ring()
 * 
 * Given some info on a ring, build the entire object structure.
 */

void Build_ring(RING *r)
{
    OBJECT         *o;
    VECTOR          pt1, pt2;

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


    o->type = T_RING;
    o->obj = r;
    o->surf = cur_surface;
    o->inter = Ring_intersect;
    o->normal = Ring_normal;

    objects[nobjects++] = o;

    /*
     * Calculate the normals and the D coefficient by various cross
     * products.
     */

    VecSub(r->point1, r->center, pt1);
    VecSub(r->point2, r->center, pt2);
    VecCross(pt1, pt2, r->normal);
    VecNormalize(&r->normal);


    r->d = -VecDot(r->normal, r->center);

    if (fabs(VecDot(r->center, r->normal) + r->d) > MIN_T)
    {
	fprintf(stderr, "%s: coordinate given in wrong order on line %d\n",
		my_name, line);
    }

    /*
     * Do some other precomp for the sake of speed.
     */

    r->o_radius2 = r->o_radius * r->o_radius;
    r->i_radius2 = r->i_radius * r->i_radius;

    /*
     * Setup the min and the max values for the bouding box.
     */

    pt1.x = pt1.y = pt1.z = r->o_radius;

    VecSub(r->center, pt1, o->b_min);
    VecAdd(r->center, pt1, o->b_max);
}

/*
 * ring_intersect()
 * 
 * Check given ring for intersection with given ray. Return TRUE if an
 * intersection takes place.
 */

int Ring_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    RING           *r;
    VECTOR          ip, tp;
    double          vo, vd;
    double          t, t2;

    r = obj->obj;

    /*
     * First check to see if this ray hit the plane which this ring
     * resides on.
     */

    vd = VecDot(ray->dir, r->normal);

    if (fabs(vd) < MIN_T)
	return (0);

    vo = VecDot(ray->pos, r->normal) + r->d;

    t = -vo / vd;
    if (t < MIN_T)
	return (0);

    /*
     * Calculate the point of intersection.
     */

    VecAddS(t, ray->dir, ray->pos, ip);

    /*
     * If the point of intersection lies between the inner and outer
     * radius, the have have a hit.
     */

    VecSub(r->center, ip, tp);
    t2 = VecDot(tp, tp);

    if (t2 < r->i_radius2 || t2 > r->o_radius2)
	return (0);

    /* we have a hit */

    inter->t = t;
    inter->obj = obj;
    inter->inside = 0;

    return (1);
}

/*
 * Ring_normal()
 * 
 * Return the normal to a ring at a given point along the surface.
 */

void Ring_normal(RING *ring, RAY *ray, VECTOR *ip, VECTOR *normal)
{
    VecCopy(ring->normal, *normal);
    if (VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}
