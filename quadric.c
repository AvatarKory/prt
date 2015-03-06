
/*
 * quadric.c - This module conatins all of the code that relates to
 * quadratics.
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

int             Quadric_intersect();
void		Quadric_normal();

/*
 * Build_quadric()
 * 
 * Given some info on a quadric, build the entire object structure.
 */

void Build_quadric(QUADRIC *q)
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


    o->type = T_QUADRIC;
    o->obj = q;
    o->surf = cur_surface;
    o->inter = Quadric_intersect;
    o->normal = Quadric_normal;

    objects[nobjects++] = o;

    /*
     * Calculate some constants that we will need in the intersect
     * routine.
     */

    q->a2 = q->a * 2.0;
    q->b2 = q->b * 2.0;
    q->c2 = q->c * 2.0;
    q->d2 = q->d * 2.0;
    q->f2 = q->f * 2.0;
    q->g2 = q->g * 2.0;
    q->i2 = q->i * 2.0;

    /*
     * Setup the min and the max values for the bouding box. For now,
     * just use what the user specifies.
     */

    o->b_min = q->min;
    o->b_max = q->max;

}

/*
 * Quadric_intersect()
 * 
 * Check given quadratic for intersection with given ray. Return TRUE if an
 * intersection takes place.
 */

int Quadric_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    QUADRIC        *q;
    VECTOR          rd, rp;
    double          aq, nbq, cq;
    double          t, disc;
    double          ka, kb;

    q = (QUADRIC *) obj->obj;

    rd = ray->dir;
    rp = ray->pos;

    /*
     * Compute Aq, Bq, Cq.
     */

    aq = rd.x * (q->a * rd.x + q->b2 * rd.y + q->c2 * rd.z) +
	rd.y * (q->e * rd.y + q->f2 * rd.z) +
	q->h * rd.z * rd.z;

    nbq = rd.x * (q->a * rp.x + q->b * rp.y + q->c * rp.z + q->d) +
	rd.y * (q->b * rp.x + q->e * rp.y + q->f * rp.z + q->g) +
	rd.z * (q->c * rp.x + q->f * rp.y + q->h * rp.z + q->i);

    cq = rp.x * (q->a * rp.x + q->b2 * rp.y + q->c2 * rp.z + q->d2) +
	rp.y * (q->e * rp.y + q->f2 * rp.z + q->g2) +
	rp.z * (q->h * rp.z + q->i2) + q->j;

    if (fabs(aq) < MIN_T)
    {
	t = -cq / (2 * nbq);
	if (t < MIN_T)
	    return (0);	/* no hit */

	inter->obj = obj;
	inter->t = t;
	inter->inside = 0;
	return (1);
    }

    /*
     * Compute the discriminator.
     */

    ka = -nbq / aq;
    kb = cq / aq;

    disc = (ka * ka) - (kb);
    if (disc < MIN_T)
	return (0);

    t = ka - sqrt(disc);

    if (t < MIN_T)
    {
	t = ka + sqrt(disc);
	if (t < MIN_T)
	    return (0);
	inter->inside = 1;
    }
    else
	inter->inside = 0;

    inter->t = t;
    inter->obj = obj;

    return (1);
}


/*
 * Quadric_normal()
 * 
 * Return the normal to a quadric at a given point along the surface.
 */

void Quadric_normal(QUADRIC *q, RAY *ray, VECTOR *ip, VECTOR *normal)
{
    normal->x = q->a * ip->x + q->b * ip->y + q->c * ip->z + q->d;
    normal->y = q->b * ip->x + q->e * ip->y + q->f * ip->z + q->g;
    normal->z = q->c * ip->x + q->f * ip->y + q->h * ip->z + q->i;

    VecNormalize(normal);

    if (VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}
