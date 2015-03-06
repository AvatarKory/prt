
/*
 * poly.c - This module conatins all of the code that relates to polygons.
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
#include <malloc.h>
#include <math.h>

#include "rt.h"
#include "externs.h"


int             Poly_intersect();
void		Poly_normal();
extern int      line;

/*
 * Build_poly()
 * 
 * Given some info on a polygon, build the entire object structure.
 */

void Build_poly(POLYGON *p)
{
    OBJECT         *o;
    VECTOR          pt1, pt2;
    int             i;

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

    o->type = T_POLYGON;
    o->obj = p;
    o->surf = cur_surface;
    o->inter = Poly_intersect;
    o->normal = Poly_normal;

    objects[nobjects++] = o;

    /*
     * Calculate the normals and the D coefficient by various cross
     * products.
     */

    VecSub(p->points[1], p->points[0], pt1);
    VecSub(p->points[2], p->points[0], pt2);
    VecCross(pt1, pt2, p->normal);
    VecNormalize(&p->normal);

    p->d = -VecDot(p->normal, p->points[0]);

    for (i = 0; i < p->npoints; i++)
    {
	if (fabs(VecDot(p->points[i], p->normal) + p->d) > MIN_T)
	{
#ifdef CHECK_COORD_ORDER
	    fprintf(stderr, "%s: coordinate given in wrong order on line %d\n",
		    my_name, line - p->npoints);
#endif
	}
    }

    /*
     * Figure out the most dominant normal.
     */

    if (fabs(p->normal.x) > fabs(p->normal.y) &&
	fabs(p->normal.x) > fabs(p->normal.z))
    {
	p->p1 = 1;
	p->p2 = 2;
    }
    else if (fabs(p->normal.y) > fabs(p->normal.x) &&
	     fabs(p->normal.y) > fabs(p->normal.z))
    {
	p->p1 = 0;
	p->p2 = 2;
    }
    else
    {
	p->p1 = 0;
	p->p2 = 1;
    }

    /*
     * Setup the min and the max values for the bouding box.
     */

    o->b_min.x = o->b_min.y = o->b_min.z = HUGE;
    o->b_max.x = o->b_max.y = o->b_max.z = -HUGE;

    for (i = 0; i < p->npoints; i++)
    {
	o->b_min.x = MIN(p->points[i].x, o->b_min.x);
	o->b_min.y = MIN(p->points[i].y, o->b_min.y);
	o->b_min.z = MIN(p->points[i].z, o->b_min.z);

	o->b_max.x = MAX(p->points[i].x, o->b_max.x);
	o->b_max.y = MAX(p->points[i].y, o->b_max.y);
	o->b_max.z = MAX(p->points[i].z, o->b_max.z);
    }
}

/*
 * Poly_intersect()
 * 
 * Check given ploy for intersection with given ray. Return TRUE if an
 * intersection takes place.
 */

int Poly_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    POLYGON        *p;
    VECTOR          ipoint;
    double          vo, vd;
    double          t, b, m;
    double          pi[3], pj[3], ip[3];
    int             qi, qj, ri, rj, i, j;
    int             n1, n2, l;


    p = obj->obj;

    /*
     * First check to see if this ray hit the plane which this polygon
     * resides on.
     */

    vd = VecDot(ray->dir, p->normal);

    if (fabs(vd) < MIN_T)
	return (0);

    vo = VecDot(ray->pos, p->normal) + p->d;

    t = -vo / vd;
    if (t < MIN_T)
	return (0);

    /*
     * OK. We now know that this ray hit the plane in which this polygon
     * resides on. Now we need to check to see if it hits the polygon. We
     * use the Jordon Curve Theorem to do this.
     */

    /* get the point of intersection on the plane */
    VecAddS(t, ray->dir, ray->pos, ipoint);
    ip[0] = ipoint.x;
    ip[1] = ipoint.y;
    ip[2] = ipoint.z;

    /* get the dominant normals */
    n1 = p->p1;
    n2 = p->p2;

    /* ok, do it */

    l = 0;
    for (i = 0; i < p->npoints; i++)
    {
	j = (i + 1) % p->npoints;
	qi = qj = ri = rj = 0;

	pi[0] = p->points[i].x;
	pi[1] = p->points[i].y;
	pi[2] = p->points[i].z;

	pj[0] = p->points[j].x;
	pj[1] = p->points[j].y;
	pj[2] = p->points[j].z;

	/* check for horizontal line and ignore them */
	if (pi[n2] == pj[n2])
	    continue;

	if (pi[n2] < ip[n2])
	    qi = 1;
	if (pj[n2] < ip[n2])
	    qj = 1;
	if (qi == qj)
	    continue;

	if (pi[n1] < ip[n1])
	    ri = 1;
	if (pj[n1] < ip[n1])
	    rj = 1;

	if (ri & rj)
	{
	    ++l;	/* crossed an edge		 */
	    continue;
	}

	if (!(rj | ri))
	    continue;

	m = (pj[n2] - pi[n2]) / (pj[n1] - pi[n1]);
	b = (pj[n2] - ip[n2]) - (m * (pj[n1] - ip[n1]));

	if ((-b / m) < MIN_T)
	    ++l;
    }

    if ((l % 2) == 0)
	return (0);

    /*
     * We have crossed an odd number of edges, that means that we have a
     * hit!!!
     */

    inter->t = t;
    inter->obj = obj;
    inter->inside = 0;

    return (1);
}


/*
 * Poly_normal()
 * 
 * Return the normal to a polygon at a given point along the surface.
 */

void Poly_normal(POLYGON *poly, RAY *ray, VECTOR *ip, VECTOR *normal)
{
    VecCopy(poly->normal, *normal);
    if (VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}
