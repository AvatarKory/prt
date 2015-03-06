/*
 * trace.c - This files contains the code which does the actuall raytracing.
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
#include <time.h>

#include "rt.h"
#include "externs.h"

COLOR           Trace_a_ray(), Background_color(), Illuminate();

/*
 * Raytrace()
 * 
 * Raytrace the entire picture.
 */

void Raytrace()
{
    RAY             ray;
    double          xr, yr, x_step, y_step, x_pw, y_pw;
    double          x_rand, y_rand;
    int             x, y;
    VECTOR          hor, ver;
    COLOR           col, scol;
    long            ts, te;
    int             l_int, l_shad, l_refl, l_refr, s;

    /* calculate the viewing frustrum. */
    VecSub(view.look_at, view.from, view.look_at);
    VecNormalize(&view.look_at);
    VecNormalize(&view.up);
    VecCross(view.up, view.look_at, hor);
    VecNormalize(&hor);	/* horizontal screen vector */
    VecCross(view.look_at, hor, ver);
    VecNormalize(&ver);	/* vertical screen vector	 */

    x_pw = x_step = 2.0 / view.x_res;
    y_pw = y_step = 2.0 / view.y_res;

    VecCopy(view.from, ray.pos);

    view.angle = tan(view.angle * M_PI / 180) / sqrt(2.0);

    l_int = l_shad = l_refl = l_refr = 0;
    time(&ts);

    /* OK, start tracing */
    yr = 1 - (y_step * (double) y_start);
    y_step = y_step * (double) y_inc;

    for (y = y_start; y < view.y_res; y += y_inc)
    {
	xr = 1;
	for (x = 0; x < view.x_res; x++)
	{
	    /*
	     * Setup the ray
	     */
	    if (sample_cnt == 1)
	    {
		VecComb(xr * view.angle, hor, yr * view.angle, ver, ray.dir);
		VecAdd(ray.dir, view.look_at, ray.dir);
		VecNormalize(&ray.dir);

		/*
		 * Trace that Ray!!
		 */

		col = Trace_a_ray(&ray, 0);
	    }
	    else
	    {
		col.r = col.g = col.b = 0.0;
		for (s = 1; s < sample_cnt; s++)
		{
		    x_rand = (xr * view.angle) + (x_pw * RAND());
		    y_rand = (yr * view.angle) + (y_pw * RAND());

		    VecComb(x_rand, hor, y_rand, ver, ray.dir);
		    VecAdd(ray.dir, view.look_at, ray.dir);
		    VecNormalize(&ray.dir);

		    /*
		     * printf("ray.dir = (%lg %lg
		     * %lg)\n", ray.dir.x, ray.dir.y,
		     * ray.dir.z);
		     */

		    scol = Trace_a_ray(&ray, 0);

		    col.r += scol.r;
		    col.g += scol.g;
		    col.b += scol.b;
		}

		col.r /= sample_cnt;
		col.g /= sample_cnt;
		col.b /= sample_cnt;
	    }

	    /*
	     * Write pixel to output file
	     */

	    Write_pixel(&col);

	    xr -= x_step;
	}

	Flush_output_file();

	yr -= y_step;

	if (verbose)
	{
	    time(&te);
	    fprintf(stderr, "\r%s: scan %d -- %ld:%02ld  i:%d  s:%d   rl:%d  rr:%d ",
		    my_name, y, (te - ts) / 60, (te - ts) % 60,
		    n_intersects - l_int, n_shadinter - l_shad,
		    n_reflect - l_refl, n_refract - l_refr);

	    l_int = n_intersects;
	    l_shad = n_shadinter;
	    l_refl = n_reflect;
	    l_refr = n_refract;

	    ts = te;
	}
    }

}


/*
 * Trace_a_ray()
 * 
 * Trace the given ray and return the resulting color.
 */

COLOR 
Trace_a_ray(RAY *ray, int n)
{
    INTERSECT       inter;
    COLOR           col;
    VECTOR          ip;
    double          t;

    ++n_rays;

    /*
     * Check to see if this ray will intersect anything. If not, then
     * return a proper background color. Else, apply the proper
     * illumination model to get the color of the object.
     */

    if (!Intersect(ray, &inter))
	return (Background_color(ray));

    ++n_intersects;

    /*
     * calculate the point of intersection and pass it to the shad
     * function
     */

    t = inter.t;
    VecAddS(t, ray->dir, ray->pos, ip);

    col = Illuminate(&inter, ray, &ip, n);

    /*
     * If colors have overflown, normalize it.
     */

    return (col);

}

/*
 * Background_color()
 * 
 * Determin what color the background should be at the given point.
 */

COLOR 
Background_color(RAY *ray)
{
    return (bkgnd.col);
}
