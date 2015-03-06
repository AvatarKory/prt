/*
 * shade.c - This module contains the illumination model.
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
#include <math.h>

#include "rt.h"
#include "externs.h"

COLOR           Illuminate(), Trace_a_ray();

/*
 * Reflect()
 * 
 * Calculate the reflected vector.
 */

void Reflect(VECTOR *v, VECTOR *norm, VECTOR *refl)
{
    double          nl;

    nl = 1 / fabs(VecDot(*v, *norm));

    VecComb(nl, *v, 2.0, *norm, *refl);
    VecNormalize(refl);
}


/*
 * Refract()
 * 
 * Calculate the refraction vector.
 */

int Refract(double n1, double  n2, VECTOR  *i, VECTOR  *n, VECTOR  *r)
{
    double          eta, c1, c2, c3;

    eta = n1 / n2;
    c1 = -VecDot(*i, *n);
    c2 = 1.0 - eta * eta * (1.0 - (c1 * c1));
    if (c2 < 0.0)
	return (0);	/* total internal reflection */

    c3 = (eta * c1) - sqrt(c2);

    VecComb(eta, *i, c3, *n, *r);
    return (1);
}


/*
 * Illuminate()
 * 
 * Apply the proper illumination model to determine the color of the object at
 * the given point.
 */

COLOR 
Illuminate(INTERSECT *inter, RAY *ray, VECTOR *ip, int n)
{
    COLOR           col, c;
    OBJECT         *obj, *scache;
    SURFACE        *surf;
    VECTOR          normal, l_dir;
    INTERSECT       test_inter;
    RAY             ray2;
    double          l_dist, incident, spec;
    double          n1, n2, intensity;
    int                l;

    /*
     * If the maximum level of recusion has been reached, then return
     * peacefully.
     */

    if (n >= MAX_LEVEL)
    {
	col.r = col.g = col.b = 0;
	return (col);
    }

    obj = inter->obj;
    surf = obj->surf;

    /* get the surface normal */
    (*obj->normal) (obj->obj, ray, ip, &normal);

    /* first set the color to ambient color */
    col = surf->c_ambient;

    /* foreach light source */
    for (l = 0; l < nlights; l++)
    {
	/*
	 * get the vector from the light source to the intersection
	 * point
	 */
	VecSub(lights[l]->pos, *ip, l_dir);

	intensity = lights[l]->intensity;

	/*
	 * Calculate the angle of incident.
	 */

	if (VecDot(normal, l_dir) >= 0)
	{
	    /*
	     * Test to see if any object is casting a shadow on
	     * this point by firing a test ray from the light
	     * source to this spot. If there is a hit and the
	     * object is not the current object, then a shadow is
	     * casted. In such a case, we don't need to calculate
	     * the diffuse color.
	     */

	    l_dist = VecNormalize(&l_dir);
	    if (shadow)
	    {
		ray2.pos = *ip;
		ray2.dir = l_dir;
		++n_shadows;

		/*
		 * If we do have a shadow cache entry for
		 * this light at this level, try that first.
		 * If it hits, then a shadow is casted. If it
		 * doesn't hit, then try all of the other
		 * primitives.
		 */

		if ((scache = lights[l]->cache[n]) != NULL)
		{
		    if ((*scache->inter) (scache, &ray2, &test_inter) &&
			inter->obj != test_inter.obj &&
			test_inter.t < l_dist - MIN_T)
		    {
			++n_shadinter;
			continue;
		    }
		}

		if (Intersect(&ray2, &test_inter) &&
		    inter->obj != test_inter.obj &&
		    test_inter.t < l_dist - MIN_T)
		{
		    lights[l]->cache[n] = test_inter.obj;
		    ++n_shadinter;
		    continue;
		}
		else
		{
		    lights[l]->cache[n] = NULL;
		}
	    }

	    incident = VecDot(normal, l_dir);
	    /* calculate the diffuse color */
	    col.r += incident * surf->c_diffuse.r * intensity;
	    col.g += incident * surf->c_diffuse.g * intensity;
	    col.b += incident * surf->c_diffuse.b * intensity;

	    /*
	     * Add some specular highlights. This is accomplished
	     * by calculating the angle of reflection and getting
	     * the dot product of it with the ray direction.
	     */

	    if (surf->spec_width != 0.0)
	    {
		Reflect(&ray->dir, &normal, &ray2.dir);
		spec = pow(VecDot(l_dir, ray2.dir), surf->spec_width);

		col.r += spec * surf->c_specular.r * intensity;
		col.g += spec * surf->c_specular.g * intensity;
		col.b += spec * surf->c_specular.b * intensity;
	    }
	}
    }

    /*
     * If reflections are enabled, calculat the reflection color.
     */

    if (reflect && surf->p_reflect != 0.0)
    {
	++n_reflect;
	ray2.pos = *ip;
	Reflect(&ray->dir, &normal, &ray2.dir);

	/*
	 * Send out reflection ray.
	 */

	c = Trace_a_ray(&ray2, n + 1);
	col.r += c.r * surf->p_reflect * surf->c_reflect.r;
	col.g += c.g * surf->p_reflect * surf->c_reflect.g;
	col.b += c.b * surf->p_reflect * surf->c_reflect.b;
    }

    /*
     * If refraction are enable, calculate the refracted color.
     */

    if (refract && surf->p_refract != 0.0)
    {
	/*
	 * determine the this ray is inside or outside the object so
	 * that we can determine the proper index of refraction to
	 * use.
	 */

	if (inter->inside)
	{
	    n1 = surf->i_refraction;
	    n2 = 1.0;
	}
	else
	{
	    n1 = 1.0;
	    n2 = surf->i_refraction;
	}

	ray2.pos = *ip;
	if (Refract(n1, n2, &ray->dir, &normal, &ray2.dir))
	{
	    ++n_refract;
	    c = Trace_a_ray(&ray2, n + 1);

	    col.r += c.r * surf->p_refract * surf->c_refract.r;
	    col.g += c.g * surf->p_refract * surf->c_refract.g;
	    col.b += c.b * surf->p_refract * surf->c_refract.b;
	}
    }

    /* return that color */
    return (col);
}

