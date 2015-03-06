/*
 * rt.h - General information file for rt
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

#include <stdlib.h>

/*
 * Define some stuff
 */

#define	MAX_LIGHTS	8	/* maximum number of light sources */
#define	MAX_PRIMS	800000	/* maximum number of primitives	 */
#define MAX_INSTANCE	64	/* maximum number of instances	   */
#define MAX_TOKENS	17
#define MIN_T		1e-12
#define MAX_LEVEL	5	/* maxmimum recursion level	   */
#define GROUP_SIZE	4
#define STACK_SIZE	512

/*
 * Object types
 */

#define T_COMPOSITE	0
#define T_POLYGON	1
#define	T_SPHERE	2
#define T_HSPHERE	3
#define T_CONE		4
#define T_RING		5
#define T_QUADRIC	6

/*
 * Instance type flags
 */

#define I_OBJECT	0	/* object type			 */
#define I_SURFACE	1	/* surface properties		 */
#define I_LIGHT		2	/* light sources		 */

/*
 * Structures
 */

/* vector */

typedef struct vector
{
	double          x;
	double          y;
	double          z;
}               VECTOR;


/* color pixel info */

typedef struct color
{
	double          r;
	double          g;
	double          b;
}               COLOR;

/* surface properties */

typedef struct surface
{
	double          p_reflect;	/* percentage of reflection	 */
	double          p_refract;	/* percentage of refraction	 */
	COLOR           c_reflect;	/* reflect color		 */
	COLOR           c_refract;	/* refract color		 */
	COLOR           c_ambient;	/* ambient color		 */
	COLOR           c_diffuse;	/* diffues color		 */
	COLOR           c_specular;	/* specular color		 */
	double          spec_width;	/* specular width factor	 */
	double          i_refraction;	/* index of refraction		 */
	double          refl_diffuse;	/* circle of diffusion in refl.	 */
	double          refr_diffuse;	/* circle of diffusion in refr.	 */
}               SURFACE;

/* composite (slab) data */

typedef struct composite
{
	int             num;	/* number of object in this group */
	struct object  *child[GROUP_SIZE];	/* pointer to members		 */
}               COMPOSITE;

/* n-point polygon */

typedef struct polygon
{
	int             npoints;/* number of points		 */
	VECTOR          normal;	/* surface normal (normalized)	 */
	double          d;	/* the D coefficient		 */
	int             p1, p2;	/* the dominant normals		 */
	VECTOR          points[1];	/* actual points		 */
}               POLYGON;


/* sphere */

typedef struct sphere
{
	VECTOR          center;	/* center of sphere		 */
	double          radius;	/* and its radius		 */
	double          radius2;/* radius * radius		 */
}               SPHERE;

/* hallow sphere */

typedef struct hsphere
{
	VECTOR          center;	/* center of sphere		 */
	double          radius;	/* and its radius		 */
	double          radius2;/* radius * radius		 */
	double          i_radius;	/* inner_radius = outer - tickness */
	double          i_radius2;	/* i_radius * i_radius		 */
}               HSPHERE;


/* cone */

typedef struct cone
{
	VECTOR          base;	/* center of base		 */
	double          base_radius;	/* base radius			 */
	double          base_d;	/* base D coefficient		 */
	VECTOR          apex;	/* center of apex		 */
	double          apex_radius;	/* apex radius			 */
	VECTOR          u;
	VECTOR          v;
	VECTOR          w;
	double          height;	/* apex - base			 */
	double          slope;	/* slope of the damn thing	 */
	double          min_d;
	double          max_d;
}               CONE;

/* ring */

typedef struct ring
{
	VECTOR          center;	/* center of ring		 */
	VECTOR          point1;	/* one point on surface		 */
	VECTOR          point2;	/* another point on surface	 */
	VECTOR          normal;	/* surface normal		 */
	double          d;	/* the D coefficient		 */
	double          o_radius;	/* outer radius			 */
	double          i_radius;	/* inner radius			 */
	double          o_radius2;	/* o_radius * o_radius		 */
	double          i_radius2;	/* i_radius * i_radius		 */
}               RING;

/*
 * General form quadratic data type.
 */

typedef struct quadric
{
	VECTOR          loc;	/* location of the quadratic	 */
	VECTOR          min;	/* minumum extent		 */
	VECTOR          max;	/* maximum extent		 */
	double          a, b, c, d, e;	/* coefficients a to J follow	 */
	double          f, g, h, i, j;
	double          a2, b2, c2, d2;	/* a2 = A * 2, etc...		 */
	double          f2, g2, i2;
}               QUADRIC;

/*
 * The OBJECT data type is built from all of the previous types.
 */

typedef struct object
{
	int             type;	/* T_* goes here		 */
	void           *obj;	/* actually point to type CONE, etc. */
	VECTOR          b_min;	/* bounding box in values	 */
	VECTOR          b_max;	/* bounding box max values	 */
	int             active;	/* TRUE if on active hit list	 */
	SURFACE        *surf;	/* object surface properties	 */
	int             (*inter) ();	/* pointer to intersect routine  */
	void            (*normal) ();	/* pointer to normal routine	 */
}               OBJECT;

/*
 * This data type contains info about object intersection
 */

typedef struct intersect
{
	OBJECT         *obj;	/* object that caused the intersect */
	double          t;	/* distance				 */
	int             inside;	/* 1 = ray is inside object		 */
}               INTERSECT;

/*
 * This date type conatins info about the image and observer.
 */

typedef struct view_info
{
	VECTOR          from;	/* observer's location			 */
	VECTOR          look_at;/* looking at here		 */
	VECTOR          up;	/* which way is up?		 */
	double          angle;	/* field of view		 */
	int             x_res;	/* x resolution			 */
	int             y_res;	/* y res			 */
}               VIEW_INFO;

/*
 * This data type contains info about background colors and cueing.
 */

typedef struct background
{
	COLOR           col;
	char            cue;
}               BACKGROUND;

/*
 * This data type contains info about lights.
 */

typedef struct light
{
	VECTOR          pos;	/* light position		 */
	COLOR           col;	/* color of light		 */
	double          intensity;	/* light intensity		 */
	OBJECT         *cache[MAX_LEVEL];	/* shadow cache			 */
}               LIGHT;

/*
 * This data type contains info about rays.
 */

typedef struct ray
{
	VECTOR          pos;	/* ray origin			 */
	VECTOR          dir;	/* ray direction		 */
}               RAY;

/*
 * Instance info holder
 */

typedef struct instance
{
	char           *name;	/* name of instance		 */
	void           *next;	/* next object pointer		 */
	void           *data;	/* points to object data	 */
	int             type;	/* I_* type			 */
	int             subtype;/* T_* type			 */
}               INSTANCE;

/* vector math stuff */

#define MakeVector(x, y, z, v)		(v).x=(x),(v).y=(y),(v).z=(z)

#define VecNegate(a)			{(a).x=0-(a).x;\
					(a).y=0-(a).y;\
					(a).z=0-(a).z;}

#define VecDot(a,b)			((a).x*(b).x+(a).y*(b).y+(a).z*(b).z)

#define VecLen(a)			(sqrt(VecDot(a,a)))

#define VecCopy(a,b)	 		{(b).x=(a).x;(b).y=(a).y;(b).z=(a).z;}

#define VecAdd(a,b,c)	 		{(c).x=(a).x+(b).x;\
			 		(c).y=(a).y+(b).y;\
			 		(c).z=(a).z+(b).z;}

#define VecSub(a,b,c)	 		{(c).x=(a).x-(b).x;\
			 		(c).y=(a).y-(b).y;\
			 		(c).z=(a).z-(b).z;}

#define VecComb(A,a,B,b,c)		{(c).x=(A)*(a).x+(B)*(b).x;\
					(c).y=(A)*(a).y+(B)*(b).y;\
			 		(c).z=(A)*(a).z+(B)*(b).z;}

#define VecAddS(A,a,b,c)	 	{(c).x=(A)*(a).x+(b).x;\
				 	(c).y=(A)*(a).y+(b).y;\
				 	(c).z=(A)*(a).z+(b).z;}

#define VecSProd(A,a,b)	 		{(b).x=(A)*(a).x;\
				 	(b).y=(A)*(a).y;\
				 	(b).z=(A)*(a).z;}

#define VecCross(a,b,c)	 		{(c).x=(a).y*(b).z-(a).z*(b).y;\
			 		(c).y=(a).z*(b).x-(a).x*(b).z;\
			 		(c).z=(a).x*(b).y-(a).y*(b).x;};

#define MIN(a, b)			((a) < (b) ? (a) : (b))
#define MAX(a, b)			((a) > (b) ? (a) : (b))

double          VecNormalize();
OBJECT         *Pop_object();

/*
 * This macro returns a random number between 0 and 1.0. It probably is not
 * portable!!
 */

#define RAND()				((double) rand() / RAND_MAX)
