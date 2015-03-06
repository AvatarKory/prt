
/*
 * hsphere.c - This module contain all of the code that relates to
 *             hallow spheres.
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

int Hsphere_intersect();
void Hsphere_normal();


/*
 * Build_hsphere()
 *
 * Given some info on a sphere object, build a complete object stucture.
 */

void Build_hsphere( HSPHERE *s)
{
    OBJECT *o;

    if(nobjects == MAX_PRIMS)
    {
	fprintf(stderr, "%s: too many objects specified\n", my_name);
	exit(1);
    }

    if((o = (OBJECT *) malloc(sizeof(OBJECT))) == NULL)
    {
	fprintf(stderr, "%s: malloc failed\n", my_name);
	exit(1);
    }

    s->radius2 = s->radius * s->radius;
    s->i_radius2 = s->i_radius * s->i_radius;
    
    o->type = T_HSPHERE;
    o->obj  = s;
    o->surf = cur_surface;
    o->inter = Hsphere_intersect;
    o->normal = Hsphere_normal;

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
 * Hsphere_intersect()
 *
 * Check given sphere for intersection with given ray. Return TRUE if
 * an intersection takes place.
 */

int Hsphere_intersect(OBJECT *obj, RAY *ray, INTERSECT *inter)
{
    HSPHERE *s;
    VECTOR oc;
    double l2oc, tca, t2hc, i_t2hc, disc, i_disc;
    double t = 0, i_t;
    int which, inside1, inside2;
    
    s = obj->obj;

    /* calculate the origin to center vector */
    
    VecSub(s->center, ray->pos, oc);
    l2oc = VecDot(oc, oc);

    /* find out the closest approach along the ray */
    tca = VecDot(oc, ray->dir);
    t2hc = s->radius2 - l2oc + (tca * tca);
    i_t2hc = s->i_radius2 - l2oc + (tca * tca);
    
    /* if the discriminator < 0, then the ray will not hit */
    if(t2hc < MIN_T)
    {
	if(i_t2hc < MIN_T)
	{
	    return (0);			/* didn't hit either one */
	}
	else
	{
	    which = 2;			/* hit the inside one	*/
	}
    }
    else
    {
	if(i_t2hc < MIN_T)		/* hit the outside one	*/
	{
	    which = 1;
	}
	else
	{
	    which = 0;			/* don't know yet	*/
	}
    }

    /* only find the sqrt root of the one we need */
    switch(which)
    {
    case 0 :		/* damm it jim! We need both! */
	disc = sqrt(t2hc);
	i_disc = sqrt(i_t2hc);
	if(l2oc > s->radius2 + MIN_T)
	{
	    inside1 = 0;
	    t = tca - disc;
	}
	else
	{
	    inside1 = 1;
	    t = tca + disc;
	}
	
	/* reverse the inside flag for this one */
	if(l2oc > s->i_radius2 + MIN_T)
	{
	    inside2 = 1;
	    i_t = tca - i_disc;
	}
	else
	{
	    inside2 = 0;
	    i_t = tca + i_disc;
	}

	/*
	 * figure out which of the spheres we hit. If we have
	 * hit both, then take the close one.
	 */

	if(t < MIN_T)	/* didn't hit the outside one, try the inside */
	{
	    if(i_t < MIN_T) /* yikes!! All this cpu time and we missed both */
	    {
		return (0);
	    }
	    else
	    {		    /* we hit this one */
		inter->t = i_t;
		inter->inside = inside2;
		inter->obj = obj;
		return (1);
	    }
	}
	else
	{
	    inter->obj = obj;
	    if(i_t < MIN_T)	/* hit the outside one only	*/
	    {
		inter->t = t;
		inter->inside = inside1;
		return (1);
	    }
	    else		/* hit both, chose the close one */
	    {
		if(i_t < t - MIN_T)
		{
		    inter->t = i_t;
		    inter->inside = inside2;
		}
		else
		{
		    inter->t = t;
		    inter->inside = inside1;
		}
		return (1);
	    }
	}
	break;

    case 1 :
	disc = sqrt(t2hc);
	/* if ray is inside object, set the inside flag */
	if(l2oc > s->radius2 + MIN_T)
	{
	    inter->inside = 0;
	    t = tca - disc;
	}
	else
	{
	    inter->inside = 1;
	    t = tca + disc;
	}
	break;

    case 2 :
	i_disc = sqrt(i_t2hc);
	/* if ray is inside object, set the inside flag to FALSE */
	if(l2oc > s->i_radius2 + MIN_T)
	{
	    inter->inside = 1;
	    t = tca - i_disc;
	}
	else
	{
	    inter->inside = 0;
	    t = tca + i_disc;
	}
	break;
    }
    

    if(t < MIN_T)
	return (0);

    inter->obj = obj;
    inter->t = t;

    return (1);
    
}


/*
 * Hsphere_normal()
 *
 * Return the normal to a sphere at a given point along the surface.
 */

void Hsphere_normal(HSPHERE *sphere, RAY *ray, VECTOR *ip, VECTOR *normal)
{

    VecSub(*ip, sphere->center, *normal);
    VecNormalize(normal);
    if(VecDot(ray->dir, *normal) >= 0)
	VecNegate(*normal);
}


 
