
/*
 * input.c - This files reads in and interperts the input file for rt.
 * 
 * Copyright (C) 1990, Kory Hamzeh
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <malloc.h>
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "rt.h"
#include "externs.h"

void Add_to_ilist(void *data, int type, int subtype);

int             Parse_from(), Parse_at(), Parse_up(), Parse_angle(), Parse_res();
int             Parse_light(), Parse_bkgnd(), Parse_surface(), Parse_cone();
int             Parse_sphere(), Parse_hallow_sphere(), Parse_poly(), Parse_ring();
int             Parse_quadric(), Parse_instance(), Parse_end_instance();
int             Parse_instanceof();

char           *Get_token();
int             iflag = 0;


struct parse_procs
{
    int             (*parse) ();
    char           *token;
};


FILE           *in_fp;
int             line;
char            line_buf[255], token[32], *info_ptr;
char           *sp = "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg";

struct parse_procs tokens[MAX_TOKENS] =
{
    {Parse_from, "from"},
    {Parse_at, "at"},
    {Parse_up, "up"},
    {Parse_angle, "angle"},
    {Parse_res, "resolution"},
    {Parse_light, "light"},
    {Parse_bkgnd, "background"},
    {Parse_surface, "surface"},
    {Parse_cone, "cone"},
    {Parse_sphere, "sphere"},
    {Parse_hallow_sphere, "hsphere"},
    {Parse_poly, "polygon"},
    {Parse_ring, "ring"},
    {Parse_quadric, "quadric"},
    {Parse_instance, "instance"},
    {Parse_end_instance, "end_instance"},
    {Parse_instanceof, "instance_of"}
};

/*
 * Syntax_error()
 * 
 * Pretty self explanatory.
 */

void Syntax_error()
{

    fprintf(stderr, "%s: syntax error on line %d in input file\n",
	    my_name, line);
    fclose(in_fp);
    exit(1);
}

/*
 * Read_input_file()
 * 
 * Read the input file given by the user. Interpert and build all of the
 * required structures.
 */

void Read_input_file(char *filename)
{
    int             i;
    char           *p;

    if (filename)
    {
	if ((in_fp = fopen(input_file, "r")) == NULL)
	{
	    fprintf(stderr, "%s: can't open input file '%s'\n",
		    my_name, input_file);
	    exit(1);
	}
    }
    else
    {
	in_fp = stdin;
    }

    line = 1;

    while (fgets(line_buf, sizeof(line_buf), in_fp))
    {

	if (verbose)
	    fprintf(stderr, "%s: parsing line %d\r", my_name, line);

	if (line_buf[0] == '#' || line_buf[0] == '\n')
	{
	    ++line;
	    continue;
	}

	if ((p = strchr(line_buf, '\n')) != NULL)
	    *p = 0;

	info_ptr = Get_token(line_buf, token);

	for (i = 0; i < MAX_TOKENS; i++)
	{
	    if (!strcmp(tokens[i].token, token))
		break;
	}


	if (i == MAX_TOKENS)
	{
	    fprintf(stderr, "%s: invalid token in line %d\n", my_name,
		    line);
	}
	else if ((*tokens[i].parse) () != 0)
	{
	    Syntax_error();
	    exit(1);
	}
	++line;
    }

    if (filename)
	fclose(in_fp);

}

/*
 * Get_token()
 * 
 * Grab the token from the given line. Return a pointer to the next token in the
 * line buffer.
 */

char           *
Get_token(char *ln, char *tk)
{
    char            c;

    /* skip leading white spaces */

    while (*ln && isspace(*ln))
	++ln;

    do
    {
	c = *ln++;
	if (c == ' ' || c == 0)
	    break;

	*tk++ = c;
    } while (1);

    *tk = 0;
    return (ln);
}

/*
 * Next_line()
 * 
 * Read the next line from the input file and bump the line counter. Croak on
 * EOF.
 */

void Next_line()
{
    ++line;
    if (!fgets(line_buf, sizeof(line_buf), in_fp))
    {
	fprintf(stderr, "%s: unexpected end-of-file\n", my_name);
	exit(1);
    }
}

/*
 * Bad_malloc()
 * 
 * Malloc has failed. Print an error message on stderr and exit.
 */

void Bad_malloc()
{

    fprintf(stderr, "%s: malloc failed.\n", my_name);
    exit(1);
}


/*
 * Parse_from()
 * 
 * Parse the from token. The format is:
 * 
 * from x y z
 * 
 */

int Parse_from()
{

    if (sscanf(info_ptr, "%lg %lg %lg", &view.from.x, &view.from.y,
	       &view.from.z) != 3)
	return (1);
    else
	return (0);
}

/*
 * Parse_at()
 * 
 * Parse the at token. The format is:
 * 
 * at x y z
 * 
 */

int Parse_at()
{

    if (sscanf(info_ptr, "%lg %lg %lg", &view.look_at.x, &view.look_at.y,
	       &view.look_at.z) != 3)
	return (1);
    else
	return (0);
}

/*
 * Parse_up()
 * 
 * Parse the up token. The format is:
 * 
 * up x y z
 * 
 */

int Parse_up()
{

    if (sscanf(info_ptr, "%lg %lg %lg", &view.up.x, &view.up.y,
	       &view.up.z) != 3)
	return (1);
    else
	return (0);
}

/*
 * Parse_angle()
 * 
 * Parse the angle token. The format is:
 * 
 * angle fov
 * 
 */

int Parse_angle()
{

    if (sscanf(info_ptr, "%lg", &view.angle) != 1)
	return (1);
    else
	return (0);
}

/*
 * Parse_res()
 * 
 * Parse the resolution token. The format is:
 * 
 * resolution x_res y_res
 * 
 */

int Parse_res()
{

    if (sscanf(info_ptr, "%d %d", &view.x_res, &view.y_res) != 2)
	return (1);
    else
	return (0);
}

/*
 * Parse_light()
 * 
 * Parse the positional light token. The format is:
 * 
 * l x y z
 * 
 */

int Parse_light()
{
    LIGHT          *l;

    if (nlights == MAX_LIGHTS)
    {
	fprintf(stderr, "%s: too many light sources defined\n", my_name);
	return (1);
    }

    if ((l = (LIGHT *) calloc(1, sizeof(LIGHT))) == NULL)
	Bad_malloc();

    if (sscanf(info_ptr, "%lg %lg %lg", &l->pos.x, &l->pos.y, &l->pos.z) != 3)
	return (1);

    lights[nlights++] = l;
    return (0);
}

/*
 * Parse_bkgnd()
 * 
 * Parse the background token. The format is:
 * 
 * b x y z c
 * 
 */

int Parse_bkgnd()
{

    if (sscanf(info_ptr, "%lg %lg %lg %c", &bkgnd.col.r, &bkgnd.col.g,
	       &bkgnd.col.b, &bkgnd.cue) != 4)
	return (1);

    if (bkgnd.cue != 'n' && bkgnd.cue != 'x' && bkgnd.cue != 'y' &&
	bkgnd.cue != 'z')
	return (1);
    else
	return (0);
}

/*
 * Parse_surface()
 * 
 * This one is a biggy. Parse the four million parameters will follow the
 * surface info token.
 */

int Parse_surface()
{
    SURFACE        *s;

    if ((s = (SURFACE *) malloc(sizeof(SURFACE))) == NULL)
	Bad_malloc();

    if (sscanf(info_ptr, sp,
	       &s->c_reflect.r, &s->c_reflect.g, &s->c_reflect.b, &s->p_reflect,
	       &s->c_refract.r, &s->c_refract.g, &s->c_refract.b, &s->p_refract,
	       &s->c_ambient.r, &s->c_ambient.g, &s->c_ambient.b,
	       &s->c_diffuse.r, &s->c_diffuse.g, &s->c_diffuse.b,
	       &s->c_specular.r, &s->c_specular.g, &s->c_specular.b,
	       &s->spec_width, &s->i_refraction) != 19)
	return (1);

    /* We need to round the spec_width to the nearest integer because
       the stupid libc pow() cannot handle non-inrerger exponents!! */

    s->spec_width = round(s->spec_width);

    /*
     * If we are in the middle of an instance, then jsut log it.
     */

    if (iflag)
	Add_to_ilist(s, I_SURFACE, 0);
    else
	cur_surface = s;
    return (0);
}

/*
 * Parse_cone()
 * 
 * Parse the cone primitive.
 */

int Parse_cone()
{
    CONE           *cd;

    if ((cd = (CONE *) malloc(sizeof(CONE))) == NULL)
	Bad_malloc();

    /* get the cone base info */
    Next_line();
    if (sscanf(line_buf, "%lg %lg %lg %lg", &cd->base.x, &cd->base.y,
	       &cd->base.z, &cd->base_radius) != 4)
	return (1);

    /* and the apex stuff */
    Next_line();
    if (sscanf(line_buf, "%lg %lg %lg %lg", &cd->apex.x, &cd->apex.y,
	       &cd->apex.z, &cd->apex_radius) != 4)
	return (1);

    if (iflag)
	Add_to_ilist(cd, I_OBJECT, T_CONE);
    else
	Build_cone(cd);
    return (0);

}

/*
 * Parse_sphere()
 * 
 * Parse the sphere primitive.
 */

int Parse_sphere()
{
    SPHERE         *s;

    if ((s = (SPHERE *) malloc(sizeof(SPHERE))) == NULL)
	Bad_malloc();

    if (sscanf(info_ptr, "%lg %lg %lg %lg", &s->center.x, &s->center.y,
	       &s->center.z, &s->radius) != 4)
	return (1);


    if (iflag)
	Add_to_ilist(s, I_OBJECT, T_SPHERE);
    else
	Build_sphere(s);
    return (0);
}

/*
 * Parse_hallow_sphere()
 * 
 * Parse the hallow sphere primitive. The format is
 * 
 * hsphere center.x center.y center.z radius tickness
 */

int Parse_hallow_sphere()
{
    HSPHERE        *s;
    double          thickness;

    if ((s = (HSPHERE *) malloc(sizeof(HSPHERE))) == NULL)
	Bad_malloc();

    if (sscanf(info_ptr, "%lg %lg %lg %lg %lg", &s->center.x, &s->center.y,
	       &s->center.z, &s->radius, &thickness) != 5)
	return (1);

    s->i_radius = s->radius - thickness;

    if (iflag)
	Add_to_ilist(s, I_OBJECT, T_HSPHERE);
    else
	Build_hsphere(s);
    return (0);
}

/*
 * Parse_poly()
 * 
 * Parse the polygon verticies info.
 */

int Parse_poly()
{
    int             np, i;
    POLYGON        *p;

    /* get the number of points */
    if (sscanf(info_ptr, "%d", &np) != 1)
	return (1);

    if ((p = (POLYGON *) malloc(sizeof(POLYGON) + (sizeof(VECTOR) * (np - 1))))
	== NULL)
	Bad_malloc();

    if (np < 3)
	return (1);

    for (i = 0; i < np; i++)
    {
	Next_line();
	if (sscanf(line_buf, "%lg %lg %lg", &p->points[i].x, &p->points[i].y,
		   &p->points[i].z) != 3)
	    return (1);
    }

    p->npoints = np;

    if (iflag)
	Add_to_ilist(p, I_OBJECT, T_POLYGON);
    else
	Build_poly(p);
    return (0);
}

/*
 * Parse_ring()
 * 
 * Parse the ring primitive. The format is:
 * 
 * ring center.x center.y center.z p1.x p1.y p1.z p2.x p2.y p2.z or ir
 */

int Parse_ring()
{
    RING           *r;

    if ((r = (RING *) malloc(sizeof(RING))) == NULL)
	Bad_malloc();

    if (sscanf(info_ptr, "%lg %lg %lg %lg %lg %lg %lg %lg %lg %lg %lg",
	       &r->center.x, &r->center.y, &r->center.z,
	       &r->point1.x, &r->point1.y, &r->point1.z,
	       &r->point2.x, &r->point2.y, &r->point2.z,
	       &r->o_radius, &r->i_radius) != 11)
	return (1);

    if (iflag)
	Add_to_ilist(r, I_OBJECT, T_RING);
    else
	Build_ring(r);
    return (0);
}

/*
 * Parse_quadric()
 * 
 * Parse the quadric data type. The format is:
 * 
 * quadric loc.x loc.y loc.z a  b  c  d  e f  g  h  i  j
 */

int Parse_quadric()
{
    QUADRIC        *q;

    /*
     * Allocate a data structure.
     */

    if ((q = (QUADRIC *) malloc(sizeof(QUADRIC))) == NULL)
	Bad_malloc();

    /*
     * Get the center of the quadratic.
     */

    if (sscanf(info_ptr, "%lg %lg %lg", &q->loc.x, &q->loc.y, &q->loc.z) != 3)
	return (1);

    /*
     * Get the min and max values.
     */

    Next_line();
    if (sscanf(line_buf, "%lg %lg %lg %lg %lg %lg",
	       &q->min.x, &q->min.y, &q->min.z,
	       &q->max.x, &q->max.y, &q->max.z) != 6)
	return (1);

    /*
     * Get the A, B, C, D, and E coefficients.
     */

    Next_line();
    if (sscanf(line_buf, "%lg %lg %lg %lg %lg", &q->a, &q->b, &q->c, &q->d,
	       &q->e) != 5)
	return (1);

    /*
     * Get the F, G, H, I, and J coefficients.
     */

    Next_line();
    if (sscanf(line_buf, "%lg %lg %lg %lg %lg", &q->f, &q->g, &q->h, &q->i,
	       &q->j) != 5)
	return (1);

    if (iflag)
	Add_to_ilist(q, I_OBJECT, T_QUADRIC);
    else
	Build_quadric(q);
    return (0);
}


/*
 * Parse_instance()
 * 
 * Start a new instance definition here.
 */

int Parse_instance()
{
    INSTANCE       *i;
    char            name[128];

    /*
     * Instances can not be nested.
     */

    if (iflag)
    {
	fprintf(stderr, "%s: instance definitions can't be nested\n.",
		my_name);
	return (1);
    }

    if (num_instance == MAX_INSTANCE)
    {
	fprintf(stderr, "%s: too many instances defined.\n",
		my_name);
	return (1);
    }


    /*
     * Get the name for this instances.
     */

    Get_token(info_ptr, name);
    if (strlen(name) < 1)
    {
	fprintf(stderr, "%s: missing or invalid instance label.\n", my_name);
	return (1);
    }

    if ((i = (INSTANCE *) malloc(sizeof(INSTANCE))) == NULL)
	Bad_malloc();

    if ((i->name = malloc(strlen(name))) == NULL)
	Bad_malloc();

    strcpy(i->name, name);
    i->type = i->subtype = -1;
    i->next = i->data = (void *) 0;

    instances[num_instance] = i;
    iflag = 1;
    return (0);
}

/*
 * Parse_end_instance()
 * 
 * Clean up and save stuff.
 */

int Parse_end_instance()
{

    /*
     * if we are not in an instance, itsa booboo.
     */

    if (!iflag)
    {
	fprintf(stderr, "%s: unexpected 'end_instance'.\n", my_name);
	return (1);
    }

    iflag = 0;
    ++num_instance;
    return (0);
}

/*
 * Parse_instanceof()
 * 
 * Build the instance requested using the given offset. Format is
 * 
 * instance_of fubar loc.x loc.y loc.z
 */

int Parse_instanceof()
{
    INSTANCE       *inst;
    SPHERE         *s;
    HSPHERE        *hs;
    POLYGON        *p, *p1;
    CONE           *c;
    RING           *r;
    QUADRIC        *q;
    VECTOR          off;
    char            name[32];
    int             i, size;

    if (iflag)
    {
	fprintf(stderr, "%s: instance_of can't be used in an instance def.\n",
		my_name);
	return (1);
    }

    /* get the instance name */
    info_ptr = Get_token(info_ptr, name);

    for (i = 0; i < num_instance; i++)
	if (!strcmp(instances[i]->name, name))
	    break;

    if (i == num_instance)
    {
	fprintf(stderr, "%s: instance '%s' was never defined.\n", my_name,
		name);
	return (1);
    }

    inst = instances[i];

    /* get the offset for this instance */
    if (sscanf(info_ptr, "%lg %lg %lg", &off.x, &off.y, &off.z) != 3)
    {
	fprintf(stderr, "%s: missing instance location.\n", my_name);
	return (1);
    }

    /* skip the first one */
    inst = inst->next;

    while (inst)
    {
	switch (inst->type)
	{
	case I_SURFACE:
	    cur_surface = (SURFACE *) inst->data;
	    break;

	case I_OBJECT:
	    switch (inst->subtype)
	    {
	    case T_POLYGON:
		p1 = (POLYGON *) inst->data;
		size = sizeof(POLYGON) + (sizeof(VECTOR) * (p1->npoints - 1));
		if ((p = (POLYGON *) malloc(size)) == NULL)
		    Bad_malloc();
		memcpy(p, p1, size);

		for (i = 0; i < p->npoints; i++)
		{
		    VecAdd(off, p->points[i], p->points[i]);
		}
		Build_poly(p);
		break;

	    case T_SPHERE:
		if ((s = (SPHERE *) malloc(sizeof(SPHERE))) == NULL)
		    Bad_malloc();

		memcpy(s, inst->data, sizeof(SPHERE));

		VecAdd(off, s->center, s->center);
		Build_sphere(s);
		break;

	    case T_HSPHERE:
		if ((hs = (HSPHERE *) malloc(sizeof(HSPHERE))) == NULL)
		    Bad_malloc();

		memcpy(hs, inst->data, sizeof(HSPHERE));
		VecAdd(off, hs->center, hs->center);
		Build_hsphere(hs);
		break;

	    case T_CONE:
		if ((c = (CONE *) malloc(sizeof(CONE))) == NULL)
		    Bad_malloc();
		memcpy(c, inst->data, sizeof(CONE));

		VecAdd(off, c->base, c->base);
		VecAdd(off, c->apex, c->apex);
		Build_cone(c);
		break;

	    case T_RING:
		if ((r = (RING *) malloc(sizeof(RING))) == NULL)
		    Bad_malloc();
		memcpy(r, inst->data, sizeof(RING));

		VecAdd(off, r->center, r->center);
		VecAdd(off, r->point1, r->point1);
		VecAdd(off, r->point2, r->point2);

		Build_ring(r);
		break;

	    case T_QUADRIC:
		if ((q = (QUADRIC *) malloc(sizeof(QUADRIC))) == NULL)
		    Bad_malloc();
		memcpy(q, inst->data, sizeof(QUADRIC));

		VecAdd(off, q->loc, q->loc);
		VecAdd(off, q->min, q->min);
		VecAdd(off, q->max, q->max);

		Build_quadric(q);
		break;

	    default:
		fprintf(stderr, "%s: internal error 01.\n", my_name);
		return (1);
	    }
	    break;

	default:
	    fprintf(stderr, "%s: internal error 02.\n", my_name);
	    return (1);
	}

	inst = inst->next;
    }

    return (0);
}


/*
 * Add_to_ilist()
 * 
 * Add the given object/surface to the end of the current instance link list.
 */

void Add_to_ilist(void *data, int type, int subtype)
{
    INSTANCE       *i1, *i2;

    /* allocate an instance structure */
    if ((i1 = (INSTANCE *) malloc(sizeof(INSTANCE))) == NULL)
	Bad_malloc();

    i2 = instances[num_instance];
    while (i2->next)
	i2 = i2->next;

    i2->next = i1;
    i1->next = (void *) 0;
    i1->data = data;
    i1->type = type;
    i1->subtype = subtype;

}
