
/*
 * bound.c
 * 
 * This module contains the code for creating a bounding box hierarchy. Most of
 * the code here has stolen from MTV's raytracer.
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

static int      axis;

/*
 * Find the most dominant axis for this group of objects.
 */

int Find_axis(int first, int last)
{
    OBJECT         *obj;
    VECTOR          mins, maxs;
    double          d, e;
    int             i, which;
    
    d = -HUGE;

    mins.x = mins.y = mins.z = HUGE;
    maxs.x = maxs.y = maxs.z = -HUGE;

    for (i = first; i < last; i++)
    {
	obj = objects[i];

	if (obj->b_min.x < mins.x)
	    mins.x = obj->b_min.x;
	if (obj->b_min.y < mins.y)
	    mins.y = obj->b_min.y;
	if (obj->b_min.z < mins.z)
	    mins.z = obj->b_min.z;
	
	if (obj->b_max.x > maxs.x)
	    maxs.x = obj->b_max.x;
	if (obj->b_max.y > maxs.y)
	    maxs.y = obj->b_max.y;
	if (obj->b_max.z > maxs.z)
	    maxs.z = obj->b_max.z;
    }
    
    e = maxs.x - mins.x;
    
    if (e > d)
    {
	d = e;
	which = 0;
    }
    
    e = maxs.y - mins.y;
    
    if (e > d)
    {
	d = e;
	which = 1;
    }
    
    e = maxs.z - mins.z;
    
    if (e > d)
    {
	d = e;
	which = 0;
    }
    
    return (which);
}

/*
 * Compslabs()
 * 
 * Compare the given slabs of the current axis.
 */


static int Compslabs(const void *p1, const void *p2)
{
    double          am = 0, bm = 0;
    OBJECT	**a = (OBJECT **) p1;
    OBJECT	**b = (OBJECT **) p2;

    switch (axis)
    {
    case 0:
	am = (*a)->b_min.x + (*a)->b_max.x;
	bm = (*b)->b_min.x + (*b)->b_max.x;
	break;

    case 1:
	am = (*a)->b_min.y + (*a)->b_max.y;
	bm = (*b)->b_min.y + (*b)->b_max.y;
	break;

    case 2:
	am = (*a)->b_min.z + (*a)->b_max.z;
	bm = (*b)->b_min.z + (*b)->b_max.z;
	break;
    }

    if (am < bm)
	return (-1);
    else if (am == bm)
	return (0);
    else
	return (1);
}


int Sort_split(int first, int last)
{
    OBJECT         *cp;
    COMPOSITE      *cd;
    int             size, i, j;
    double          dmin, dmax;
    int             m;

    axis = Find_axis(first, last);

    size = last - first;

    qsort((char *) (objects + first), 
	  size, 
	  sizeof(OBJECT *), 
	  Compslabs);

    if (size <= GROUP_SIZE)
    {
	/* build a box to contain them */

	cp = (OBJECT *) malloc(sizeof(OBJECT));
	cp->type = T_COMPOSITE;
	cd = (COMPOSITE *) malloc(sizeof(COMPOSITE));
	cd->num = size;

	for (i = 0; i < size; i++)
	{
	    cd->child[i] = objects[first + i];
	}

	dmin = HUGE;
	dmax = -HUGE;

	for (j = 0; j < size; j++)
	{
	    if (cd->child[j]->b_min.x < dmin)
		dmin = cd->child[j]->b_min.x;
	    if (cd->child[j]->b_max.x > dmax)
		dmax = cd->child[j]->b_max.x;
	}

	cp->b_min.x = dmin;
	cp->b_max.x = dmax;

	dmin = HUGE;
	dmax = -HUGE;

	for (j = 0; j < size; j++)
	{
	    if (cd->child[j]->b_min.y < dmin)
		dmin = cd->child[j]->b_min.y;
	    if (cd->child[j]->b_max.y > dmax)
		dmax = cd->child[j]->b_max.y;
	}

	cp->b_min.y = dmin;
	cp->b_max.y = dmax;

	dmin = HUGE;
	dmax = -HUGE;

	for (j = 0; j < size; j++)
	{
	    if (cd->child[j]->b_min.z < dmin)
		dmin = cd->child[j]->b_min.z;
	    if (cd->child[j]->b_max.z > dmax)
		dmax = cd->child[j]->b_max.z;
	}

	cp->b_min.z = dmin;
	cp->b_max.z = dmax;


	cp->obj = (void *) cd;
	root = cp;

	if (nobjects < MAX_PRIMS)
	{
	    objects[nobjects++] = cp;
	    return (1);
	}
	else
	{
	    fprintf(stderr, "%s: too many primitives, max is %d\n",
		    my_name, MAX_PRIMS);
	    exit(0);
	}
    }
    else
    {
	m = (first + last) / 2;
	Sort_split(first, m);
	Sort_split(m, last);
	return (0);
	}
}

/*
 * Build_bounding_slabs()
 * 
 * This function attempts to use median cut to generate tighter bounding volumes
 * than the old code...
 */

void Build_bounding_slabs()
{
    int             low = 0;
    int             high;

    high = nobjects;
    while (Sort_split(low, high) == 0)
    {
	low = high;
	high = nobjects;
    }
}

